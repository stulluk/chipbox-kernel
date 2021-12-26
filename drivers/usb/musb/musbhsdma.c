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
 * DMA implementation for high-speed controllers.
 * $Revision: 1.1.1.1 $
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <asm/types.h>
#include "plat_arc.h"
#include <linux/mem_define.h>
#define MGC_HSDMA

/****************************** CONSTANTS ********************************/

#define MGC_O_HSDMA_BASE    	0x200
#define MGC_O_HSDMA_PKRCNT_BASE 0x300
#define MGC_O_HSDMA_INTR    	0x200

#define MGC_O_HSDMA_CONTROL 	4
#define MGC_O_HSDMA_ADDRESS 	8
#define MGC_O_HSDMA_COUNT   	0xc

#define MGC_HSDMA_CHANNEL_OFFSET(_bChannel, _bOffset) (MGC_O_HSDMA_BASE + (_bChannel << 4) + _bOffset)
#define MGC_HSDMA_PKTCNT_OFFSET(_bEnd) (MGC_O_HSDMA_PKRCNT_BASE + (_bEnd<< 2) )
/* control register (16-bit): */
#define MGC_S_HSDMA_ENABLE		0
#define MGC_S_HSDMA_TRANSMIT		1
#define MGC_S_HSDMA_MODE1		2
#define MGC_S_HSDMA_IRQENABLE		3
#define MGC_S_HSDMA_ENDPOINT		4
#define MGC_S_HSDMA_BUSERROR		8
#define MGC_S_HSDMA_BURSTMODE		9
#define MGC_M_HSDMA_BURSTMODE	(3 << MGC_S_HSDMA_BURSTMODE)
#define MGC_HSDMA_BURSTMODE_UNSPEC  	0
#define MGC_HSDMA_BURSTMODE_INCR4   	1
#define MGC_HSDMA_BURSTMODE_INCR8   	2
#define MGC_HSDMA_BURSTMODE_INCR16  	3

/******************************** TYPES **********************************/

struct _MGC_HsDmaController;

typedef struct {
        MGC_DmaChannel Channel;
        struct _MGC_HsDmaController* pController;
        uint32_t dwStartAddress;
        uint32_t dwCount;
        uint16_t wMaxPacketSize;
        uint8_t bIndex;
        uint8_t bEnd;
        uint8_t bProtocol;
        uint8_t bTransmit;
} MGC_HsDmaChannel;

#ifndef MGC_HSDMA_CHANNELS
#define MGC_HSDMA_CHANNELS		2
#endif
typedef struct _MGC_HsDmaController {
        MGC_DmaController Controller;
        MGC_HsDmaChannel aChannel[MGC_HSDMA_CHANNELS];
        MGC_pfDmaChannelStatusChanged pfDmaChannelStatusChanged;
        void* pDmaPrivate;
        uint8_t* pCoreBase;
        uint8_t bChannelCount;
        uint8_t bmUsedChannels;
	 uint8_t __iomem * pDMABuf;	//point to the free iomemory FB1_REGION
	 uint8_t * pCoreBuf;  //point to the buffer from the USBD Core
	 MGC_LinuxCd* pThis;
} MGC_HsDmaController;

/******************************* FORWARDS ********************************/
/*static */extern void mgc_start_next_urb(MGC_LinuxCd* pThis, uint8_t bEnd);
static uint8_t MGC_HsDmaStartController(void* pPrivateData);

static uint8_t MGC_HsDmaStopController(void* pPrivateData);

static MGC_DmaChannel* MGC_HsDmaAllocateChannel(void* pPrivateData,
		uint8_t bLocalEnd, uint8_t bTransmit, uint8_t bProtocol,
		uint16_t wMaxPacketSize);

static void MGC_HsDmaReleaseChannel(MGC_DmaChannel* pChannel);

static uint8_t* MGC_HsDmaAllocateBuffer(MGC_DmaChannel* pChannel,
                                        uint32_t dwLength);

static uint8_t MGC_HsDmaReleaseBuffer(MGC_DmaChannel* pChannel,
                                      uint8_t* pBuffer);

static uint8_t MGC_HsDmaProgramChannel(MGC_DmaChannel* pChannel,
                                       uint16_t wPacketSize, uint8_t bMode,
                                       const uint8_t* pBuffer, uint32_t dwLength,void* pThis);

static MGC_DmaChannelStatus MGC_HsDmaGetChannelStatus(
        MGC_DmaChannel* pChannel);

static uint8_t MGC_HsDmaControllerIsr(void* pPrivateData);

static MGC_DmaController* MGC_HsNewDmaController(
        MGC_pfDmaChannelStatusChanged pfDmaChannelStatusChanged,
        void* pDmaPrivate,
        uint8_t* pCoreBase);

static void MGC_HsDestroyDmaController(MGC_DmaController* pController);

/******************************* GLOBALS *********************************/

MGC_DmaControllerFactory MGC_HdrcDmaControllerFactory = {
        0x300,
        MGC_HsNewDmaController,
        MGC_HsDestroyDmaController
};

/****************************** FUNCTIONS ********************************/

#ifdef MGC_DMA
static uint8_t MGC_HsDmaStartController(void* pPrivateData)
{
        /* nothing to do */
        return TRUE;
}

static uint8_t MGC_HsDmaStopController(void* pPrivateData)
{
        /* nothing to do */
        return TRUE;
}

static MGC_DmaChannel* MGC_HsDmaAllocateChannel(void* pPrivateData,
                uint8_t bLocalEnd, uint8_t bTransmit, uint8_t bProtocol,
                uint16_t wMaxPacketSize)
{
        uint8_t bBit;
        MGC_DmaChannel* pChannel = NULL;
        MGC_HsDmaChannel* pImplChannel = NULL;
        MGC_HsDmaController* pController = (MGC_HsDmaController*)pPrivateData;
        for (bBit = 0; bBit < MGC_HSDMA_CHANNELS; bBit++) {
                if (!(pController->bmUsedChannels & (1 << bBit))) {
                        pController->bmUsedChannels |= (1 << bBit);
                        pImplChannel = &(pController->aChannel[bBit]);
                        pImplChannel->pController = pController;
                        pImplChannel->wMaxPacketSize = wMaxPacketSize;
                        pImplChannel->bIndex = bBit;
                        pImplChannel->bEnd = bLocalEnd;
                        pImplChannel->bProtocol = bProtocol;
                        pImplChannel->bTransmit = bTransmit;
                        pChannel = &(pImplChannel->Channel);
                        pChannel->pPrivateData = pImplChannel;
                        pChannel->bStatus = MGC_DMA_STATUS_FREE;
                        pChannel->dwMaxLength = 0x10000;
                        pChannel->bDesiredMode = 1; /*modify by LAND */
                        break;
                }
        }
        return pChannel;
}

static void MGC_HsDmaReleaseChannel(MGC_DmaChannel* pChannel)
{
        MGC_HsDmaChannel* pImplChannel = (MGC_HsDmaChannel*)pChannel->pPrivateData;

        pImplChannel->pController->bmUsedChannels &= ~(1 << pImplChannel->bIndex);
        pImplChannel->Channel.bStatus = MGC_DMA_STATUS_FREE;
}

static uint8_t* MGC_HsDmaAllocateBuffer(MGC_DmaChannel* pChannel,
                                        uint32_t dwLength)
{
        /* do nothing here, we save the data to the buffer offered by URB */
		printk("LAND>%s\n",__FUNCTION__);
        return 0;
}

static uint8_t MGC_HsDmaReleaseBuffer(MGC_DmaChannel* pChannel,
                                      uint8_t* pBuffer)
{
        /* do nothing here, we save the data to the buffer offered by URB */
		printk("LAND>%s\n",__FUNCTION__);
        return 0;
}
static uint8_t MGC_HsDmaProgramChannel(MGC_DmaChannel* pChannel,
                                       uint16_t wPacketSize, uint8_t bMode,
                                       const uint8_t* pBuffer, uint32_t dwLength,void* pThis)
{
        MGC_HsDmaChannel* pImplChannel = (MGC_HsDmaChannel*)pChannel->pPrivateData;
        MGC_HsDmaController* pController = pImplChannel->pController;
        uint8_t* pBase = pController->pCoreBase;
        uint16_t wCsr = (pImplChannel->bEnd << MGC_S_HSDMA_ENDPOINT) |
		        (1 << MGC_S_HSDMA_ENABLE) | 
		        (MGC_HSDMA_BURSTMODE_INCR16 << MGC_S_HSDMA_BURSTMODE);
        uint8_t bChannel = pImplChannel->bIndex;
	pController->pCoreBuf=(uint8_t*)pBuffer;
	pController->pThis=(MGC_LinuxCd*)pThis;
  	if (bMode) {
                wCsr |= 1 << MGC_S_HSDMA_MODE1;
        }

        if (pImplChannel->bTransmit) {
                wCsr |= 1 << MGC_S_HSDMA_TRANSMIT;
        }

        wCsr |= 1 << MGC_S_HSDMA_IRQENABLE;

        /* address/count */
  
	MGC_Write32(pBase,
                    MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_ADDRESS),
                    (uint32_t)pBuffer /*virt_to_phys(pBuffer)*/); /* Already physical address */	
        MGC_Write32(pBase,
                    MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_COUNT), dwLength);	
        MGC_Write8(pBase,
                    MGC_HSDMA_PKTCNT_OFFSET(pImplChannel->bEnd), (dwLength + wPacketSize-1)/wPacketSize);
		/* control (this should start things) */
        pChannel->dwActualLength = dwLength;

 
        pImplChannel->dwStartAddress = (uint32_t)pBuffer /*virt_to_phys(pBuffer)*/;
        pImplChannel->dwCount = dwLength;
        MGC_Write16(pBase,
                    MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_CONTROL), wCsr);
	
        return TRUE;
}

static MGC_DmaChannelStatus MGC_HsDmaGetChannelStatus(MGC_DmaChannel* pChannel)
{
        uint32_t dwAddress;
        MGC_HsDmaChannel* pImplChannel = (MGC_HsDmaChannel*)pChannel->pPrivateData;
        MGC_HsDmaController* pController = pImplChannel->pController;
        uint8_t* pBase = pController->pCoreBase;
        uint8_t bChannel = pImplChannel->bIndex;
        uint16_t wCsr = MGC_Read16(pBase,
                                   MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_CONTROL));

        if (wCsr & (1 << MGC_S_HSDMA_BUSERROR)) {
                return MGC_DMA_STATUS_BUS_ABORT;
        }

        /* most DMA controllers would update the count register for simplicity... */
        dwAddress = MGC_Read32(pBase, MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_ADDRESS));
                    MGC_Read32(pBase, MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_COUNT));
        if (dwAddress < (pImplChannel->dwStartAddress + pImplChannel->dwCount)) {
                return MGC_DMA_STATUS_BUSY;
        }

        return MGC_DMA_STATUS_FREE;
}


static uint8_t MGC_HsDmaControllerIsr(void* pPrivateData)
{
        uint8_t bChannel;
        uint16_t wCsr;
	 unsigned long flags;
	 uint8_t* pBase_reg ;
	MGC_DmaChannel* pDmaChannel;
	MGC_DmaController* pDmaController;
		
        MGC_HsDmaChannel* pImplChannel;
        MGC_HsDmaController* pController = (MGC_HsDmaController*)pPrivateData;
	 MGC_LinuxCd* pThis=pController->pThis;
	 MGC_LinuxLocalEnd* pEnd ;
	  struct urb* pUrb;
        uint8_t* pBase = pController->pCoreBase;
        uint8_t bIntr = MGC_Read8(pBase, MGC_O_HSDMA_INTR);
	unsigned int last_small_packet;

	 unsigned int nPipe ;
        unsigned int nOut ;
	unsigned int usec_cnt;
  spin_lock_irqsave(&pThis->Lock, flags);
        if (!bIntr)
	{
		printk("LAND>%s:bIntr=%d,now return\n",__FUNCTION__,bIntr);
	return FALSE;
	}

        for (bChannel = 0; bChannel < MGC_HSDMA_CHANNELS; bChannel++) {
                if (bIntr & (1 << bChannel)) {
                        pImplChannel = (MGC_HsDmaChannel*)&(pController->aChannel[bChannel]);
                        wCsr = MGC_Read16(pBase,
                                          MGC_HSDMA_CHANNEL_OFFSET(bChannel, MGC_O_HSDMA_CONTROL));
                        if (wCsr & (1 << MGC_S_HSDMA_BUSERROR)) {
                                pImplChannel->Channel.bStatus = MGC_DMA_STATUS_BUS_ABORT;
                                printk(" DELME: %s:%d ---> BUS_ABORT(%d) \n", __FUNCTION__, __LINE__, wCsr);
                        } else {
                             pImplChannel->Channel.bStatus = MGC_DMA_STATUS_FREE;
			     pController->pfDmaChannelStatusChanged(pController->pDmaPrivate, pImplChannel->bEnd, pImplChannel->bTransmit);
			     pEnd = &(pThis->aLocalEnd[ pImplChannel->bEnd]);
			     pUrb = pEnd->pUrb;
			     if(pUrb)
			     {
				      	nPipe = pUrb->pipe;
      					nOut = usb_pipeout(nPipe);
					pUrb->actual_length=pImplChannel->Channel.dwActualLength ;
					pUrb->status=USB_ST_NOERROR;
					pBase_reg = (uint8_t*)pThis->pRegs;
					MGC_SelectEnd(pBase_reg, pImplChannel->bEnd);
					if(nOut) 
					{
						/* Sent last small packet */
						last_small_packet = MGC_ReadCsr16(pBase_reg, MGC_O_HDRC_TXMAXP, pImplChannel->bEnd);
						last_small_packet = pImplChannel->Channel.dwActualLength % last_small_packet;
                				wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd);
                				if(last_small_packet) {
									wCsr |= MGC_M_TXCSR_TXPKTRDY | MGC_M_TXCSR_MODE;
                					MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd, wCsr);
						}
						/* Wait last packet to be sent */
						usec_cnt = 0;
						do {	
							usec_cnt++;						
							udelay(1); /* USB 2.0 send 512bytes < 10us, USB 1.1 < 400us */
							wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd);
						} while((wCsr & MGC_M_TXCSR_TXPKTRDY) && (usec_cnt < 10000));	
						if(usec_cnt == 10000)
							printk(KERN_ERR "ERROR: USB spent too much time on one packet\n");

						/* see if we need to send a null */
                     if ( (!last_small_packet) && (pUrb->transfer_flags & USB_ZERO_PACKET) ) {
							uint16_t wIntrTxE;
							wIntrTxE = MGC_Read16(pBase, MGC_O_HDRC_INTRTXE);
                			MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE & ~(1 << pImplChannel->bEnd));
								
							wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd);
							wCsr &= ~(MGC_M_TXCSR_DMAENAB | MGC_M_TXCSR_DMAMODE);	/* no DMA here */
                			wCsr |=  MGC_M_TXCSR_TXPKTRDY | MGC_M_TXCSR_MODE;
                			MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd, wCsr);

							usec_cnt = 0;
							do {	
								usec_cnt++;						
								udelay(1); /* USB 2.0 send 512bytes < 10us, USB 1.1 < 400us */
								wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd);
							} while((wCsr & MGC_M_TXCSR_TXPKTRDY) && (usec_cnt < 10000));	
							if(usec_cnt == 10000)
								printk(KERN_ERR "ERROR: USB spent too much time on null packet sending\n");

							MGC_Read16(pBase, MGC_O_HDRC_INTRTX);/* read clear int flag */
							MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE);
                     }

       						wCsr= MGC_ReadCsr16(pBase_reg, MGC_O_HDRC_TXCSR, pImplChannel->bEnd);
				        	usb_settoggle(pUrb->dev, pEnd->bEnd, 1,
                                      					       	(wCsr & MGC_M_TXCSR_H_DATATOGGLE) ? 1 : 0);
					}
					else 
					{
						wCsr = MGC_ReadCsr16(pBase_reg, MGC_O_HDRC_RXCSR, pImplChannel->bEnd);
						usb_settoggle(pUrb->dev, pEnd->bEnd, 0,
						                                      	(wCsr& MGC_M_RXCSR_H_DATATOGGLE) ? 1 : 0);
						/* Clear last small packet */
						MGC_WriteCsr16(pBase_reg, MGC_O_HDRC_RXCSR, pImplChannel->bEnd, wCsr & ~(MGC_M_RXCSR_RXPKTRDY|MGC_M_RXCSR_AUTOCLEAR|MGC_M_RXCSR_AUTOREQ));
					}

					MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd, MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, pImplChannel->bEnd)&(~MGC_M_TXCSR_MODE));
	
					if (pUrb->complete)
					{
                        			pUrb->hcpriv=NULL;
                        			pUrb->complete(pUrb, NULL);
                			}	
				}	
				
		 		pDmaController = pThis->pDmaController;
	       			pDmaChannel = pEnd->pDmaChannel;
				pDmaController->pfDmaReleaseChannel(pDmaChannel);
				pEnd->pDmaChannel = NULL;

				spin_lock(&pEnd->Lock);
				mgc_start_next_urb(pThis, pImplChannel->bEnd);
				spin_unlock(&pEnd->Lock);
				spin_unlock_irqrestore(&pThis->Lock, flags);
                        }

                }
				
        }
		
	return TRUE;
}

#endif	/* MGC_HSDMA */

static MGC_DmaController* MGC_HsNewDmaController(
        MGC_pfDmaChannelStatusChanged pfDmaChannelStatusChanged,
        void* pDmaPrivate, uint8_t* pCoreBase)
{
        MGC_DmaController* pResult = NULL;

#ifdef MGC_DMA
        MGC_HsDmaController* pController = (MGC_HsDmaController*)kmalloc(sizeof(MGC_HsDmaController), GFP_KERNEL);
        if (pController) {
                memset(pController, 0, sizeof(MGC_HsDmaController));
                pController->bChannelCount = MGC_HSDMA_CHANNELS;
                pController->pfDmaChannelStatusChanged = pfDmaChannelStatusChanged;
                pController->pDmaPrivate = pDmaPrivate;
                pController->pCoreBase = pCoreBase;
                pController->Controller.pPrivateData = pController;
                pController->Controller.pfDmaStartController = MGC_HsDmaStartController;
                pController->Controller.pfDmaStopController = MGC_HsDmaStopController;
                pController->Controller.pfDmaAllocateChannel = MGC_HsDmaAllocateChannel;
                pController->Controller.pfDmaReleaseChannel = MGC_HsDmaReleaseChannel;
                pController->Controller.pfDmaAllocateBuffer = MGC_HsDmaAllocateBuffer;
                pController->Controller.pfDmaReleaseBuffer = MGC_HsDmaReleaseBuffer;
                pController->Controller.pfDmaProgramChannel = MGC_HsDmaProgramChannel;
                pController->Controller.pfDmaGetChannelStatus = MGC_HsDmaGetChannelStatus;
                pController->Controller.pfDmaControllerIsr = MGC_HsDmaControllerIsr;
		  pController->pDMABuf= ioremap(FB1_REGION,4096);	
		  memset( pController->pDMABuf,0xAA,4096);
                pResult = &(pController->Controller);
        }
#endif

        return pResult;
}

static void MGC_HsDestroyDmaController(MGC_DmaController* pController)
{
#ifdef MGC_DMA
        MGC_HsDmaController* pHsController = (MGC_HsDmaController*)pController->pPrivateData;

        if (pHsController) {
                pHsController->Controller.pPrivateData = NULL;
                kfree(pHsController);
        }
#endif
}

