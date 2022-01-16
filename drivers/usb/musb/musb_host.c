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
 * Host functions
 * $Revision: 1.1.1.1 $
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/smp_lock.h>
#include <linux/list.h>

#include <asm/uaccess.h>

#include <linux/mem_define.h>
#ifdef CONFIG_USB_DEBUG
#define DEBUG
#else
#undef DEBUG
#endif

#include <linux/usb.h>
#include <linux/version.h>

/****************************** CONSTANTS ********************************/

/** how much to "scale" response timeouts */
#define MGC_MAX_RETRIES         4

/******************************* FORWARDS ********************************/

static uint8_t mgc_find_first_one(unsigned int nValue);
/*static */void mgc_start_next_urb(MGC_LinuxCd* pThis, uint8_t bEnd);
static void mgc_start_urb(MGC_LinuxCd* pThis, uint8_t bEnd);
static int mgc_find_end(MGC_LinuxCd* pThis, struct urb* pUrb);
static uint8_t mgc_packet_rx(MGC_LinuxCd* pThis, uint8_t bEnd,
                             uint8_t bIsochError);
static void mgc_hdrc_program_end(MGC_LinuxCd* pThis, uint8_t bEnd,
                                 struct urb* pUrb, unsigned int nOut,
                                 uint8_t* pBuffer, uint32_t dwLength);
static uint8_t mgc_hdrc_service_host_default(MGC_LinuxCd* pThis,
                uint16_t wCount, struct urb* pUrb);
//static void mgc_hdrc_dump_regs(MGC_LinuxCd* pThis, uint8_t bEnd);


//static void mgc_hdrc_dump_regs(MGC_LinuxCd* pThis, uint8_t bEnd)
//{
	/* Do what ? I don't know, just clean warning,  added by sunbin 20081120 */
//}

/**************************************************************************
 * Glue for virtual root hub
 **************************************************************************/

/* see virthub.h */
static void mgc_set_port_power(void* pPrivateData, uint8_t bPortIndex,
                               uint8_t bPower)
{
        unsigned long flags;
        MGC_LinuxCd* pThis = (MGC_LinuxCd*)pPrivateData;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

        spin_lock_irqsave(&pThis->Lock, flags);

        if (bPower) {
                DEBUG_CODE(1, printk(KERN_INFO "%s: Root port power on\n", __FUNCTION__); )
                mgc_hdrc_start(pThis);
        } else {
                DEBUG_CODE(1, printk(KERN_INFO "%s: Root port power off\n", __FUNCTION__); )
                mgc_hdrc_stop(pThis);
        }

        spin_unlock_irqrestore(&pThis->Lock, flags);
}

/* see virthub.h */
static void mgc_set_port_enable(void* pPrivateData, uint8_t bPortIndex,
                                uint8_t bEnable)
{
        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
        if (bEnable) {
                DEBUG_CODE(1, printk(KERN_INFO "%s: Root port enabled\n", __FUNCTION__); )
        } else {
                DEBUG_CODE(1, printk(KERN_INFO "%s: Root port disabled\n", __FUNCTION__); )
        }
}

/**
 * Timer completion callback to finish resume handling started in ISR
 */
static void mgc_hdrc_resume_off(unsigned long pParam)
{
        uint8_t power;
        MGC_LinuxCd* pThis = (MGC_LinuxCd*)pParam;
        void* pBase = pThis->pRegs;

        power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
        MGC_Write8(pBase, MGC_O_HDRC_POWER, power & ~MGC_M_POWER_RESUME);
}

/* see virthub.h */
static void mgc_set_port_suspend(void* pPrivateData, uint8_t bPortIndex,
                                 uint8_t bSuspend)
{
        uint8_t power;
        MGC_LinuxCd* pThis = (MGC_LinuxCd*)pPrivateData;
        void* pBase = pThis->pRegs;

        power = MGC_Read8(pBase, MGC_O_HDRC_POWER);

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

        if (bSuspend) {
                DEBUG_CODE(1, printk(KERN_INFO "%s: Root port suspended\n", \
                                     __FUNCTION__); )
                MGC_Write8(pBase, MGC_O_HDRC_POWER, power | MGC_M_POWER_SUSPENDM);
        } else if (power & MGC_M_POWER_SUSPENDM) {
                DEBUG_CODE(1, printk(KERN_INFO "%s: Root port resumed\n", \
                                     __FUNCTION__); )

                power &= ~(MGC_M_POWER_SUSPENDM | MGC_M_POWER_RESUME);
                MGC_Write8(pBase, MGC_O_HDRC_POWER,
                           power | MGC_M_POWER_RESUME);
                mgc_set_timer(pThis, mgc_hdrc_resume_off, (unsigned long)pThis, 20);
        }
}

/* see virthub.h */
static void mgc_set_port_reset(void* pPrivateData, uint8_t bPortIndex,
                               uint8_t bReset)
{
        uint8_t power, devctl;
        MGC_LinuxCd* pThis = (MGC_LinuxCd*)pPrivateData;
        void* pBase = pThis->pRegs;

        devctl = MGC_Read8(pBase, MGC_O_HDRC_DEVCTL);

        power = MGC_Read8(pBase, MGC_O_HDRC_POWER) & 0xf0;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
        if (bReset) {
                DEBUG_CODE(2, printk(KERN_INFO "%s: Starting root port reset...\n", \
                                     __FUNCTION__); )
//                mgc_ignore_disconnect = TRUE;
                MGC_Write8(pBase, MGC_O_HDRC_POWER, power | MGC_M_POWER_RESET);
                mgc_set_timer(pThis, mgc_hdrc_reset_off, (unsigned long)pThis, 50);
        } else if (power & MGC_M_POWER_RESET) {
                DEBUG_CODE(2, printk(KERN_INFO "%s: Root port reset stopped\n", \
                                     __FUNCTION__); )
                MGC_Write8(pBase, MGC_O_HDRC_POWER, power & ~MGC_M_POWER_RESET);
        }
}


/**************************************************************************
 * Linux HCD functions
 **************************************************************************/

/**
 * Find first (lowest-order) 1 bit
 * @param nValue value in which to search
 * @return bit position (0 could mean no bit; caller should check)
 */
static uint8_t mgc_find_first_one(unsigned int nValue)
{
        unsigned int nWork = nValue;
        uint8_t bResult;

        for (bResult = 0; bResult < 32; bResult++) {
                if (nWork & 1) {
                        return bResult;
                }
                nWork >>= 1;
        }
        return bResult;
}

/* Root speed need to be translated (addapted)
 */
static uint8_t mgc_translate_virtualhub_speed(uint8_t source)
{
        uint8_t speed=2;

        switch ( source ) {
        case 3:
                speed=0;
                break;
        case 2:
                speed=1;
                break;
        }

        return speed;
}

/**
 * Timer completion callback to turn off reset and get connection speed
 */
static void mgc_hdrc_reset_off(unsigned long param)
{
        uint8_t power;
        unsigned long flags;
        MGC_LinuxCd* pThis = (MGC_LinuxCd*)param;
        void* pBase = pThis->pRegs;

        spin_lock_irqsave(&pThis->Lock, flags);
//        mgc_ignore_disconnect = FALSE;
        DEBUG_CODE(2, printk(KERN_INFO "%s: Stopping root port reset...\n", \
                             __FUNCTION__); )

        power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
        MGC_Write8(pBase, MGC_O_HDRC_POWER, power & ~MGC_M_POWER_RESET);

        /* check for high-speed and set in root device if so */
        power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
        if (power & MGC_M_POWER_HSMODE) {
                DEBUG_CODE(1, printk(KERN_INFO "%s: high-speed device connected\n", \
                                     __FUNCTION__); )
                pThis->bRootSpeed = 1;
        }

        MGC_VirtualHubPortResetDone(&(pThis->RootHub), 0,
                                    mgc_translate_virtualhub_speed(pThis->bRootSpeed));

        spin_unlock_irqrestore(&pThis->Lock, flags);
}


/**
 * return the buffer associated to an urb (map it too)
 */
static inline uint8_t* get_urb_buffer(struct urb* pUrb)
{
        uint8_t *pBuffer=NULL;

        if ( !pUrb ) {
                return NULL;
        }
   /*     if (pUrb->transfer_flags & URB_NO_TRANSFER_DMA_MAP) {

                pBuffer=(void*)phys_to_virt(pUrb->transfer_dma);

        } else */{
                pBuffer=pUrb->transfer_buffer;

        }

 //       if ( !pBuffer ) {
 //               pBuffer=(void*)phys_to_virt(pUrb->transfer_dma);

 //       }

        return pBuffer;
}

/**
 * Program an HDRC endpoint as per the given URB
 * @param pThis instance pointer
 * @param bEnd local endpoint
 * @param pURB URB pointer
 * @param nOut zero for Rx; non-zero for Tx
 * @param pBuffer buffer pointer
 * @param dwLength how many bytes to transmit or expect to receive
 */
static void mgc_hdrc_program_end(MGC_LinuxCd* pThis, uint8_t bEnd,
                                 struct urb* pUrb, unsigned int nOut,
                                 uint8_t* pBuffer, uint32_t dwLength)
{

        uint16_t wFrame;
        uint16_t wCsr, wLoadCount;
	uint16_t wIntrTxE, wIntrRxE;
        struct usb_device* pParent;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;
        unsigned int nPipe = pUrb->pipe;
        uint16_t wPacketSize = usb_maxpacket(pUrb->dev, nPipe, usb_pipeout(nPipe));
        uint8_t bIsBulk = usb_pipebulk(nPipe);
        uint8_t bAddress = (uint8_t)usb_pipedevice(nPipe);
        uint8_t bRemoteEnd = (uint8_t)usb_pipeendpoint(nPipe);
        uint8_t bSpeed = (uint8_t)pUrb->dev->speed;
        uint8_t bInterval = (uint8_t)pUrb->interval;
        MGC_LinuxLocalEnd* pEnd = &(pThis->aLocalEnd[bEnd]);
        uint8_t bStdType = 0;
        uint8_t bHubAddr = 0;
        uint8_t bHubPort = 0;
        uint8_t reg = 0;
        uint8_t bIsMulti = FALSE;
        uint8_t bDone = FALSE;
        unsigned long flags;
#ifdef MGC_DMA
	uint8_t bDmaOk = FALSE;
	uint8_t bWantDMA=FALSE;
	uint8_t bReFlags=FALSE;
	MGC_DmaChannel* pDmaChannel;
	MGC_DmaController* pDmaController;
#endif

        DEBUG_CODE(2, printk(KERN_INFO "=> %s(bEnd=%d, pUrb=%lx, transfer_dma=%lx, nOut=%d, dwLength=%d)\n", \
                             __FUNCTION__, bEnd, (unsigned long)pUrb, pUrb->transfer_dma, nOut, dwLength); )
        /* NOTE: there is always a parent due to the virtual root hub */
        /* parent hub address */
        pParent = pUrb->dev->parent;
        bHubAddr = (uint8_t)pParent->devnum;
        if ((bHubAddr == pThis->RootHub.bAddress) || !pParent->parent) {
                /* but not if parent is our virtual root hub */
                bHubAddr = 0;
        }

        /* parent port */
        /* if tt pointer, use its info */
        if (pUrb->dev->tt) {
                bHubPort = (uint8_t)pUrb->dev->ttport;
#ifdef HAS_USB_TT_MULTI
                bIsMulti = (uint8_t)pUrb->dev->tt->multi;
#endif
        }

        DEBUG_CODE(1, \
                   printk(KERN_INFO "%s: end %d, device %d, parent %d, port %d, multi-tt: %d, speed:%d\n", \
                          __FUNCTION__, bEnd, pUrb->dev->devnum, bHubAddr, \
                          bHubPort, bIsMulti, pUrb->dev->speed ); )

        /* prepare endpoint registers according to flags */
        if (usb_pipeisoc(nPipe)) {
                bStdType = 1;
                if (pUrb->interval > 16) {
                        /* correct interval */
                        bInterval = mgc_find_first_one(pUrb->interval);
                }
                if (bInterval < 1) {
                        bInterval = 1;
                }
        } else if (usb_pipeint(nPipe)) {
                bStdType = 3;
                if ((USB_SPEED_HIGH == bSpeed) && (pUrb->interval > 255)) {
                        /* correct interval for high-speed */
                        bInterval = mgc_find_first_one(pUrb->interval);
                }
                if (bInterval < 1) {
                        bInterval = 1;
                }
        } else if (bIsBulk) {
                bStdType = 2;
                if (pUrb->interval > 16) {
                        /* correct interval */
                        bInterval = mgc_find_first_one(pUrb->interval);
                }
        } else {
                nOut = 1;

                if (pUrb->interval > 16) {
                        /* correct interval */
                        bInterval = mgc_find_first_one(pUrb->interval);
                }
        }
        reg = bStdType << 4;
        /* really NAKlimit in this case */
        if (bInterval < 2) {
                bInterval = 16;
        }

        reg |= (bRemoteEnd & 0xf);
        if (pThis->bIsMultipoint) {
                switch (bSpeed) {
                case USB_SPEED_LOW:
                        reg |= 0xc0;
                        break;
                case USB_SPEED_FULL:
                        reg |= 0x80;
                        break;
                default:
                        reg |= 0x40;
                }
        }

        if (bIsBulk && pThis->bBulkSplit) {
                wLoadCount = min((uint32_t)pEnd->wMaxPacketSizeTx, dwLength);
        } else {
                wLoadCount = min((uint32_t)wPacketSize, dwLength);
        }

#ifdef MGC_DMA
	pDmaController = pThis->pDmaController;
	pDmaChannel = pEnd->pDmaChannel;
    	if(bIsBulk&&(dwLength>32))	/* Skip CBW(31bytes) and CSW(13bytes) */
	   	bWantDMA=WANTS_DMA(pUrb);
	else
	 	bWantDMA=FALSE;
#endif

        /* finally the hardware access begins */
        spin_lock_irqsave(&pThis->Lock, flags);

        MGC_SelectEnd(pBase, bEnd);
	wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);

        if (nOut) {
                /* transmit */
                /* disable interrupt in case we flush */
                wIntrTxE = MGC_Read16(pBase, MGC_O_HDRC_INTRTXE);
                MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE & ~(1 << bEnd));
                if (bEnd) {
                        /* general endpoint */
                        /* if not ready, flush and restore data toggle */
                        if (!pEnd->bIsReady && pThis->bIsMultipoint) {
                                pEnd->bIsReady = TRUE;
                                /* twice in case of double packet buffering */
                                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd,
                                               MGC_M_TXCSR_FLUSHFIFO | MGC_M_TXCSR_CLRDATATOG);
                                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd,
                                               MGC_M_TXCSR_FLUSHFIFO | MGC_M_TXCSR_CLRDATATOG);
                                /* data toggle */
                                wCsr |= MGC_M_TXCSR_H_WR_DATATOGGLE;
                                if (usb_gettoggle(pUrb->dev, pEnd->bEnd, 1)) {
                                        wCsr |= MGC_M_TXCSR_H_DATATOGGLE;
                                } else {
                                        wCsr &= ~MGC_M_TXCSR_H_DATATOGGLE;
                                }
                        }
                } else {
                        pThis->bEnd0Stage = MGC_END0_START;
                        /* endpoint 0: just flush */
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, bEnd,
                                       MGC_M_CSR0_FLUSHFIFO);
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, bEnd,
                                       MGC_M_CSR0_FLUSHFIFO);
                }

                if (pThis->bIsMultipoint) {
                        /* target addr & hub addr/port */
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_TXFUNCADDR),
                                   bAddress);
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_TXHUBADDR),
                                   bIsMulti ? 0x80 | bHubAddr : bHubAddr);
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_TXHUBPORT),
                                   bHubPort);
                        /* also, try Rx */
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_RXFUNCADDR),
                                   bAddress);
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_RXHUBADDR),
                                   bIsMulti ? 0x80 | bHubAddr : bHubAddr);
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_RXHUBPORT),
                                   bHubPort);
                } else {
                        /* non-multipoint core */
                        MGC_Write8(pBase, MGC_O_HDRC_FADDR, bAddress);
                }

                /* protocol/endpoint/interval/NAKlimit */
                if (bEnd) {
                        MGC_WriteCsr8(pBase, MGC_O_HDRC_TXTYPE, bEnd, reg);
                        if (bIsBulk && pThis->bBulkSplit) {
                                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXMAXP, bEnd,
                                               wPacketSize |
                                               ((pEnd->wMaxPacketSizeTx / wPacketSize) - 1) << 11);
                        } else {
                                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXMAXP, bEnd, wPacketSize);
                        }
                        MGC_WriteCsr8(pBase, MGC_O_HDRC_TXINTERVAL, bEnd, bInterval);

                } else {
                        MGC_WriteCsr8(pBase, MGC_O_HDRC_NAKLIMIT0, 0, bInterval);
                        if (pThis->bIsMultipoint) {
                                MGC_WriteCsr8(pBase, MGC_O_HDRC_TYPE0, 0, reg & 0xc0);
                        }
                }
/************************add dma ops  **********************/
if(bWantDMA)
{
         //while(pDmaChannel);
         if( pDmaChannel && usb_stat[0] == 0x30 ){
			if( pDmaController )
				pDmaController->pfDmaReleaseChannel(pDmaChannel);
			pEnd->pDmaChannel = NULL;
			spin_unlock_irqrestore(&pThis->Lock, flags);
			return;
		}

	if(pDmaController && !pDmaChannel) {
			pDmaChannel = pEnd->pDmaChannel = pDmaController->pfDmaAllocateChannel(
				pDmaController->pPrivateData, bEnd, nOut ? TRUE : FALSE,
				bStdType, wPacketSize);
		}

	if(pDmaChannel) {
		/* we got a channel */
		//if(dwLength < wPacketSize)
		//	pDmaChannel->bDesiredMode = 0; /* Use DMA0 for less than 1 MaxP */

		wCsr |= (MGC_M_TXCSR_DMAENAB | MGC_M_TXCSR_MODE |
			(pDmaChannel->bDesiredMode ? (MGC_M_TXCSR_AUTOSET| MGC_M_TXCSR_DMAMODE) : 0));
		pDmaChannel->dwActualLength = 0L;
		pEnd->dwRequestSize = min(dwLength, pDmaChannel->dwMaxLength);
		bDmaOk = pDmaController->pfDmaProgramChannel(pDmaChannel,
			wPacketSize, pDmaChannel->bDesiredMode, (uint8_t*)(pUrb->transfer_dma) /*pBuffer*/,
			pEnd->dwRequestSize,(void*)pThis);
		if(bDmaOk) {
			/* success; avoid loading FIFO using CPU */
			wLoadCount = 0;
		} else {
			/* programming failed; give up */
			pDmaController->pfDmaReleaseChannel(pDmaChannel);
			pEnd->pDmaChannel = NULL;
		}
	}
	 MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wCsr);
    }

	if (wLoadCount) {
	   pEnd->dwRequestSize = wLoadCount;
	    mgc_hdrc_load_fifo(pThis->pRegs, bEnd, wLoadCount, pBuffer);
	}

	/* re-enable interrupt */
	//wIntrTxE=0xf;
	MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE);

	/* write CSR */
	if (bEnd) {
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wCsr | MGC_M_TXCSR_MODE);
	}
	} else
	{ /* receive */
                /* if programmed for Tx, be sure it is ready for re-use */
				  MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wCsr & (~MGC_M_TXCSR_MODE));
#if	0
                if (wCsr & MGC_M_TXCSR_MODE) { /*this env is in priphrial mode ,not take place in orion now*/
                        pEnd->bIsReady = FALSE;
                        if (wCsr & MGC_M_TXCSR_FIFONOTEMPTY) {
                                /* this shouldn't happen */
                                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd,
                                               MGC_M_TXCSR_FRCDATATOG);
                                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd,
                                               MGC_M_TXCSR_FRCDATATOG);
                                printk(KERN_ERR
                                       "%s: switching end %d to Rx but Tx FIFO not empty\n",
                                       __FUNCTION__, bEnd);
                        }
                        /* clear mode (and everything else) to enable Rx */
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, 0);
                }
#endif
                /* grab Rx residual if any */
                wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
                if (wCsr & MGC_M_RXCSR_RXPKTRDY) {
                      DEBUG_CODE(1, printk( "LAND>%s: end %d Rx residual\n", \
                                             __FUNCTION__, bEnd); )
                        bDone = mgc_packet_rx(pThis, bEnd, FALSE);

			   if(bReFlags)
			   	{
				          if (bDone) {
						          /* save data toggle if re-using */
					                if (usb_pipebulk(nPipe)) {
					                        /* we re-use bulk, so re-programming required */
					                        pEnd->bIsReady = FALSE;
					                        /* release claim if borrowed */
					                        if ((bEnd != pThis->bBulkRxEnd) &&
					                                        (pThis->bBulkTxEnd != pThis->bBulkRxEnd)) {
					                                pEnd->bIsClaimed = FALSE;
					                        }
					                        /* save data toggle */
					                        usb_settoggle(pUrb->dev, pEnd->bEnd, 0,
					                                      (wCsr& MGC_M_RXCSR_H_DATATOGGLE) ? 1 : 0);

					                }
					                if (pUrb) {
					                        /* set status */
					                        pUrb->status =USB_ST_NOERROR;
					                        if (pUrb->complete) {
												printk("mgc_hdrc_program_end 1\n");
					                                pUrb->hcpriv=NULL;
									pUrb->complete(pUrb, NULL);
					                        }
					                }

					                spin_lock(&pEnd->Lock);
					                mgc_start_next_urb(pThis, bEnd);
					                spin_unlock(&pEnd->Lock);
					        } else {
					                /* continue by clearing RxPktRdy and setting ReqPkt */
					                wCsr&= ~MGC_M_RXCSR_RXPKTRDY;
					                wCsr|= MGC_M_RXCSR_H_REQPKT;
					                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsr);
				bReFlags=FALSE;
				return;
			       }
			   	}
                }

                /* address */
                if (pThis->bIsMultipoint) {/*orion 1200 use this config--LAND*/
                        /* target addr & hub addr/port */
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_RXFUNCADDR),
                                   bAddress);
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_RXHUBADDR),
                                   bIsMulti ? 0x80 | bHubAddr : bHubAddr);
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_RXHUBPORT),
                                   bHubPort);
                        /* also, try Tx */
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_TXFUNCADDR),
                                   bAddress);
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_TXHUBADDR),
                                   bIsMulti ? 0x80 | bHubAddr : bHubAddr);
                        MGC_Write8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_TXHUBPORT),
                                   bHubPort);
                } else {
                        /* non-multipoint core */
                        MGC_Write8(pBase, MGC_O_HDRC_FADDR, bAddress);
                }

                /* protocol/endpoint/interval/NAKlimit */
                if (bEnd) {
                        MGC_WriteCsr8(pBase, MGC_O_HDRC_RXTYPE, bEnd, reg);
                        if (bIsBulk && pThis->bBulkCombine) {
                                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXMAXP, bEnd,
                                               wPacketSize |
                                               ((pEnd->wMaxPacketSizeRx / wPacketSize) - 1) << 11);
                        } else {
                                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXMAXP, bEnd, wPacketSize);
                        }
                        MGC_WriteCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd, bInterval);
                } else if (pThis->bIsMultipoint) {
                        MGC_WriteCsr8(pBase, MGC_O_HDRC_TYPE0, 0, reg & 0xc0);
                }

                /* first time or re-program and shared FIFO, flush & clear toggle */
                if (!pEnd->bIsReady && pEnd->bIsSharedFifo) {
                        /* twice in case of double packet buffering */
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd,
                                       MGC_M_RXCSR_FLUSHFIFO | MGC_M_RXCSR_CLRDATATOG);
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd,
                                       MGC_M_RXCSR_FLUSHFIFO | MGC_M_RXCSR_CLRDATATOG);
                        //pEnd->bIsReady = TRUE;
                }

                /* program data toggle if possibly switching use */
                if (!pEnd->bIsReady && pThis->bIsMultipoint) {
                	wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
                        wCsr |= MGC_M_RXCSR_H_WR_DATATOGGLE;
                        if (usb_gettoggle(pUrb->dev, pEnd->bEnd, 0)) {
                                wCsr |= MGC_M_RXCSR_H_DATATOGGLE;
                        }
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsr);
                }

                /* kick things off */
                if (bEnd && !bDone) {
                        wCsr = MGC_M_RXCSR_H_REQPKT;
                        if (usb_pipeint(nPipe)) {
                                wCsr |= MGC_M_RXCSR_DISNYET;
                        }
	/* if appropriate, get a DMA channel */

	 if(bWantDMA) {
		/* candidate for DMA */
		//while(pDmaChannel);
		if( pDmaChannel && usb_stat[0] == 0x30 ){
			if( pDmaController )
				pDmaController->pfDmaReleaseChannel(pDmaChannel);
			pEnd->pDmaChannel = NULL;
			spin_unlock_irqrestore(&pThis->Lock, flags);
			return;
		}

		if(pDmaController && !pDmaChannel) {
			pDmaChannel = pEnd->pDmaChannel = pDmaController->pfDmaAllocateChannel(
				pDmaController->pPrivateData, bEnd, nOut ? TRUE : FALSE,
				bStdType, wPacketSize);
		}
		if(pDmaChannel) {
			/* we got a channel */
			//if(dwLength < wPacketSize)
			//	pDmaChannel->bDesiredMode = 0; /* Use DMA0 for less than 1 MaxP */
			wCsr |=  MGC_M_RXCSR_DMAENAB;
			wCsr &= ~MGC_M_RXCSR_DMAMODE;	       /* Here is the DMA **Request** mode 0 */
			if(dwLength >= wPacketSize) {
				wCsr |= MGC_M_RXCSR_AUTOREQ;
				if(pDmaChannel->bDesiredMode)
					wCsr |= MGC_M_RXCSR_AUTOCLEAR;
			} else {
				wCsr &= ~(MGC_M_RXCSR_AUTOREQ | MGC_M_RXCSR_AUTOCLEAR);
			}
			pDmaChannel->dwActualLength = 0L;
			pEnd->dwRequestSize = min(dwLength, pDmaChannel->dwMaxLength);
			bDmaOk = pDmaController->pfDmaProgramChannel(pDmaChannel,
				wPacketSize, pDmaChannel->bDesiredMode, (uint8_t*)(pUrb->transfer_dma) /*pBuffer*/,
				pEnd->dwRequestSize,(void*)pThis);
			if(!bDmaOk) {
				/* programming failed; give up */
				pDmaController->pfDmaReleaseChannel(pDmaChannel);
				pEnd->pDmaChannel = NULL;
			}

		}
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsr);
	} else {
                wIntrRxE = MGC_Read16(pBase, MGC_O_HDRC_INTRRXE);
                //MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrRxE & ~(1 << bEnd));
                MGC_Write16(pBase, MGC_O_HDRC_INTRRXE, wIntrRxE | (1 << bEnd));

		MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wCsr);
#if	0
		usec_cnt = 0;
		do {
			udelay(1);
			usec_cnt++;
			wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
		} while((!(wCsr & MGC_M_RXCSR_RXPKTRDY)) && (usec_cnt <= 16));

		if(!(wCsr & MGC_M_RXCSR_RXPKTRDY)) {	/* handling current Urb later */
                	wIntrRxE = MGC_Read16(pBase, MGC_O_HDRC_INTRRXE);
                	MGC_Write16(pBase, MGC_O_HDRC_INTRRXE, wIntrRxE | (1 << bEnd));
		} else {				/* handling current Urb immediately */
			wIntrRxValue = MGC_Read16(pBase, MGC_O_HDRC_INTRRX);
			for(i = 0; i < 15; i++)
				if(wIntrRxValue & (1 << i))
					mgc_hdrc_service_rx_ready(pThis, i);
		}
#endif
	}

        }

    }
        DEBUG_CODE(2, printk(KERN_INFO "%s:%d: done, unlocking pThis\n", \
                             __FUNCTION__, __LINE__); )


        spin_unlock_irqrestore(&pThis->Lock, flags);

        if (nOut && !bWantDMA) {	/* During DMA, TX will be auto started */
                /* determine if the time is right for a periodic transfer */
                if ( usb_pipeisoc(nPipe) || usb_pipeint(nPipe) ) {
                        DEBUG_CODE(3, printk(KERN_ERR "%s:%d: check whether there's still time for periodic Tx\n",
                                             __FUNCTION__, __LINE__ ); );

                        pEnd->dwIsoPacket = 0;
                        wFrame = MGC_Read16(pBase, MGC_O_HDRC_FRAME);
                        if ((pUrb->transfer_flags & USB_ISO_ASAP) ||
                                        (wFrame >= pUrb->start_frame)) {
                                pEnd->dwWaitFrame = 0;
                                mgc_hdrc_start_tx(pThis, bEnd);
                        } else {
                                pEnd->dwWaitFrame = pUrb->start_frame;
                                /* enable SOF interrupt so we can count down */
                                MGC_Write8(pBase, MGC_O_HDRC_INTRUSBE, 0xff);
                        }
                } else {
                        DEBUG_CODE(3, printk(KERN_ERR "%s:%d: starting Tx\n",
                                             __FUNCTION__, __LINE__ ); );
                        mgc_hdrc_start_tx(pThis, bEnd);
                }
        }

}

/**
 * Try to stop traffic on the given local endpoint
 */
static void mgc_hdrc_stop_end(MGC_LinuxCd* pThis, uint8_t bEnd)
{
        uint16_t wCsr;
        unsigned long flags;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;
        const uint8_t reg=(bEnd)?MGC_O_HDRC_RXCSR:MGC_O_HDRC_CSR0;

        spin_lock_irqsave(&pThis->Lock, flags);
        MGC_SelectEnd(pBase, bEnd);
        wCsr = MGC_ReadCsr16(pBase, reg, bEnd);
        wCsr &= (bEnd)?~MGC_M_RXCSR_H_REQPKT:~MGC_M_CSR0_H_REQPKT;
        MGC_WriteCsr16(pBase, reg, bEnd, wCsr);
        spin_unlock_irqrestore(&pThis->Lock, flags);
}

/**
 * Start transmit.
 * Caller is responsible for locking shared resources.
 *
 * @param pThis instance pointer
 * @param bEnd local endpoint
 */
static void mgc_hdrc_start_tx(MGC_LinuxCd* pThis, uint8_t bEnd)
{
        uint16_t wCsr;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

        /* NOTE: no locks here; caller should lock */
        MGC_SelectEnd(pBase, bEnd);

         if (bEnd) {
		/* To reduce the Bulk CBW/CSW interrupts,
                   we are try to finish small packet in polling */
                //wIntrTxE = MGC_Read16(pBase, MGC_O_HDRC_INTRTXE);
                //MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE & ~(1 << bEnd));

                wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);
				  wCsr &= ~(MGC_M_TXCSR_DMAENAB | MGC_M_TXCSR_DMAMODE);	/* no DMA here */
                wCsr |=  MGC_M_TXCSR_TXPKTRDY | MGC_M_TXCSR_MODE;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wCsr);
#if	0
		usec_cnt = 0;
		do {
			udelay(1);
			usec_cnt++;
			wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);
		} while((wCsr & MGC_M_TXCSR_TXPKTRDY) && (usec_cnt <= 16));

		if(wCsr & MGC_M_TXCSR_TXPKTRDY) {	/* handling current Urb later */
                	wIntrTxE = MGC_Read16(pBase, MGC_O_HDRC_INTRTXE);
                	MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, wIntrTxE | (1 << bEnd));
		} else {				/* handling current Urb immediately */
			wIntrTxValue = MGC_Read16(pBase, MGC_O_HDRC_INTRTX);
			for(i = 0; i < 15; i++)
				if(wIntrTxValue & (1 << i))
					mgc_hdrc_service_tx_avail(pThis, i);
		}
#endif
        } else {
                wCsr = MGC_M_CSR0_H_NO_PING | MGC_M_CSR0_H_SETUPPKT | MGC_M_CSR0_TXPKTRDY;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsr);
        }
}

/**
 * Find an endpoint for the given pipe
 *
 * @param pThis instance pointer
 * @param pURB URB pointer
 * @return suitable local endpoint
 * @return -1 if nothing appropriate
 */
static int mgc_find_end(MGC_LinuxCd* pThis, struct urb* pUrb)
{
        MGC_LinuxLocalEnd* pEnd;
        uint8_t bDirOk, bTrafficOk, bSizeOk, bExact;
        int nEnd;
        int32_t dwDiff;
        uint16_t wBestDiff = 0xffff;
        uint16_t wBestExactDiff = 0xffff;
        int nBestEnd = -1;
        int nBestExactEnd = -1;
        unsigned int nPipe = pUrb->pipe;
        unsigned int nOut = usb_pipeout(nPipe);
        uint16_t wPacketSize = usb_maxpacket(pUrb->dev, nPipe, usb_pipeout(nPipe));
        uint8_t bEnd = usb_pipeendpoint(nPipe);
        uint8_t bIsBulk = usb_pipebulk(nPipe);
        uint8_t bAddress = (uint8_t)usb_pipedevice(nPipe);

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
        /* control is always EP0, and can always be queued */
        if (usb_pipecontrol(pUrb->pipe)) {
                DEBUG_CODE(3, printk(KERN_INFO "%s: is a control pipe use ep0\n", \
                                     __FUNCTION__); )

                return 0;
        }

        /* use a reserved one for bulk if any */
        if (bIsBulk) {
                if (nOut && pThis->bBulkTxEnd) {
                        DEBUG_CODE(3, printk(KERN_INFO "%s: use the bulk tx end\n", \
                                             __FUNCTION__); )
                        return pThis->bBulkTxEnd;
                } else if (!nOut && pThis->bBulkRxEnd) {
                        DEBUG_CODE(3, printk(KERN_INFO "%s: use the bulk rx end\n", \
                                             __FUNCTION__); )
                        return pThis->bBulkRxEnd;
                }
        }

        /* scan, remembering exact match and best match (bulk only) */
        for (nEnd = 1; nEnd < pThis->bEndCount; nEnd++) {
                pEnd = &(pThis->aLocalEnd[nEnd]);
                /* consider only if direction is possible */
                bDirOk = (nOut && pEnd->wMaxPacketSizeTx) ||
                         (!nOut && pEnd->wMaxPacketSizeRx);
                /* consider only if size is possible (in the given direction) */
                bSizeOk = (nOut && (pEnd->wMaxPacketSizeTx >= wPacketSize)) ||
                          (!nOut && (pEnd->wMaxPacketSizeRx >= wPacketSize));
                /* consider only traffic type */
                bTrafficOk = (usb_pipetype(nPipe) == pEnd->bTrafficType);

                if (bDirOk && bSizeOk) {
                        /* convenient computations */
                        dwDiff = nOut ? (pEnd->wMaxPacketSizeTx - wPacketSize) :
                                 (pEnd->wMaxPacketSizeRx - wPacketSize);
                        bExact = bTrafficOk && (pEnd->bEnd == bEnd) &&
                                 (pEnd->bAddress == bAddress);

                        /* bulk: best size match not claimed (we only claim periodic) */
                        if (bIsBulk && !pEnd->bIsClaimed && (wBestDiff > dwDiff)) {
                                wBestDiff = (uint16_t)dwDiff;
                                nBestEnd = nEnd;
                                /* prefer end already in right direction (to avoid flush) */
                                if ((wBestExactDiff > dwDiff) && (nOut == (int)pEnd->bIsTx)) {
                                        wBestExactDiff = (uint16_t)dwDiff;
                                        nBestExactEnd = nEnd;
                                }
                        } else if (!bIsBulk && (nEnd != pThis->bBulkTxEnd) &&
                                        (nEnd != pThis->bBulkRxEnd)) {
                                /* periodic: exact match if present; otherwise best unclaimed */
                                if (bExact) {
                                        nBestExactEnd = nEnd;
                                        break;
                                } else if (!pEnd->bIsClaimed && (wBestDiff > dwDiff)) {
                                        wBestDiff = (uint16_t)dwDiff;
                                        nBestEnd = nEnd;
                                }
                        }
                }

        }

        DEBUG_CODE(2, printk(KERN_INFO \
                             "%s(out=%d, size=%d, proto=%d, addr=%d, end=%d, urb=%lx) = %d\n", \
                             __FUNCTION__, nOut, wPacketSize, usb_pipetype(nPipe), \
                             bAddress, bEnd, (unsigned long)pUrb, \
                             (nBestExactEnd >= 0) ? nBestExactEnd : nBestEnd); )


        return (nBestExactEnd >= 0) ? nBestExactEnd : nBestEnd;
}

/**
 * Receive a packet
 * @requires pThis->Lock locked
 * @return 0 if URB is complete
 */
static uint8_t mgc_packet_rx(MGC_LinuxCd* pThis, uint8_t bEnd,
                             uint8_t bIsochError)
{
        uint16_t wRxCount;
        uint16_t wLength;
        uint8_t* pBuffer;
        uint16_t wCsr;
        uint8_t bDone = FALSE;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;
        MGC_LinuxLocalEnd* pEnd = &(pThis->aLocalEnd[bEnd]);
        struct urb* pUrb = pEnd->pUrb;
        int nPipe=0;
        void *buffer=NULL;

        wRxCount = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCOUNT, bEnd);

        DEBUG_CODE(2, printk(KERN_INFO "%s: end %d RxCount=%04x\n", \
                             __FUNCTION__, bEnd, wRxCount); )

#ifdef MGC_PARANOID
        if ( !pUrb || ((pUrb->transfer_buffer_length - pEnd->dwOffset)<0) )  {
                printk(KERN_ERR "ERROR during Rx: pUrb=%p, pUrb->transfer_buffer_length=%d pEnd->dwOffset=%d\n",
                       pUrb, pUrb->transfer_buffer_length, pEnd->dwOffset );
                return TRUE;
        }
#endif

        nPipe=pUrb->pipe;

        buffer = get_urb_buffer(pUrb);
        DEBUG_CODE(3, printk(KERN_INFO "%s: pUrb->transfer_flags=0x%x transfer_buffer=%p\n", \
                             __FUNCTION__, pUrb->transfer_flags, buffer); )

        if ( !buffer ) { // abort the transfer
                printk(KERN_ERR "Rx requested but no buffer was given, aborting\n");
                return TRUE;
        }


        /* unload FIFO */
        if ( usb_pipeisoc(nPipe) ) {
                /* isoch case */
                pBuffer = buffer + pUrb->iso_frame_desc[pEnd->dwIsoPacket].offset;
                wLength = min((unsigned int)wRxCount,
                              pUrb->iso_frame_desc[pEnd->dwIsoPacket].length);
                pUrb->actual_length += wLength;
                /* update actual & status */
                pUrb->iso_frame_desc[pEnd->dwIsoPacket].actual_length = wLength;
                if (bIsochError) {
                        pUrb->iso_frame_desc[pEnd->dwIsoPacket].status = USB_ST_CRC;
                        pUrb->error_count++;
                } else {
                        pUrb->iso_frame_desc[pEnd->dwIsoPacket].status = USB_ST_NOERROR;
                }

                /* see if we are done */
                bDone = (++pEnd->dwIsoPacket >= pUrb->number_of_packets);
        } else {
                /* non-isoch */
                pBuffer = buffer + pEnd->dwOffset;
                wLength = min((unsigned int)wRxCount,
                              pUrb->transfer_buffer_length - pEnd->dwOffset);
                pUrb->actual_length += wLength;
                pEnd->dwOffset += wLength;

                /* see if we are done */
                bDone = (pEnd->dwOffset >= pUrb->transfer_buffer_length) ||
                        (wRxCount < pEnd->wPacketSize);
        }

	mgc_hdrc_unload_fifo(pBase, bEnd, wLength, pBuffer);

        if (wRxCount <= wLength) {
                wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd,
                               wCsr & ~MGC_M_RXCSR_RXPKTRDY);
                wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);
        }

        return bDone;
}

/**
 * Start the next URB on an endpoint. Wants the _endpoint_ to be locked.
 * It might call mgc_start_urb tha needs pThis to be locked as well.
 *
 * @param pThis instance pointer
 * @param pUrb pointer to just-d URB
 * @param bEnd local endpoint
 */
/*static */void mgc_start_next_urb(MGC_LinuxCd* pThis, uint8_t bEnd)
{
        int nPipe;
        MGC_LinuxLocalEnd* pEnd = &(pThis->aLocalEnd[bEnd]);
        struct urb* pUrb = pEnd->pUrb;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s(pThis=%p, %d)\n", __FUNCTION__, pThis, bEnd); )

        /* check for linked URB */
        if (!list_empty(&pEnd->urb_list)) {
                if (pEnd->pUrb) {
                        usb_put_urb(pEnd->pUrb);
                }

                pUrb = list_entry(pEnd->urb_list.next, struct urb, urb_list);
                if (pUrb) {
						   if((&pUrb->urb_list)->prev == (void *)0x00200200)
							 	(&pUrb->urb_list)->prev = &pEnd->urb_list;

                        list_del(&pUrb->urb_list);
                        if (pUrb) nPipe = pUrb->pipe;
                        /*if (pUrb && USB_ENDPOINT_HALTED(pUrb->dev, usb_pipeendpoint(nPipe),
                                                        usb_pipeout(nPipe))) {
                                pUrb = NULL;
                        }*/
                }

                pEnd->pUrb = pUrb;

                DEBUG_CODE(1, printk(KERN_INFO "%s: dequeued URB %lx\n", \
                                     __FUNCTION__, (unsigned long)pEnd->pUrb); )

        } else {
                if (pEnd->pUrb) {
                        usb_put_urb(pEnd->pUrb);
                }
                pEnd->pUrb = NULL;
                DEBUG_CODE(1, printk(KERN_INFO "%s: end %d idle\n", \
                                     __FUNCTION__, bEnd); )
        }

        if (pEnd->pUrb) {
        		if( usb_stat[0] == 0x30 ){

							return;
				}
                mgc_start_urb(pThis, bEnd);
        }
}

/**
 * Start the current URB on an endpoint; wants ep to be
 * locked and pThis to be locked as well; end must be claimed from
 * the caller
 */
static void mgc_start_urb(MGC_LinuxCd* pThis, uint8_t bEnd)
{
        //uint16_t wFrame;
        uint32_t dwLength;
        void* pBuffer;
        MGC_LinuxLocalEnd* pEnd = &(pThis->aLocalEnd[bEnd]);
        struct urb* pUrb = pEnd->pUrb;
        unsigned int nPipe = pUrb->pipe;
        unsigned int nOut = usb_pipeout(nPipe);
        uint8_t bAddress = (uint8_t)usb_pipedevice(nPipe);
        uint8_t bRemoteEnd = (uint8_t)usb_pipeendpoint(nPipe);
        uint16_t wPacketSize = usb_maxpacket(pUrb->dev, nPipe, usb_pipeout(nPipe));

        DEBUG_CODE(2, printk(KERN_INFO "=> %s (bOut = %d)\n", __FUNCTION__, nOut); )

        /* if no root device, assume this must be it */
        if (!pThis->pRootDevice) {
                pThis->pRootDevice = pUrb->dev;
                switch (pThis->bRootSpeed) {
                case 1:
                        pThis->pRootDevice->speed = USB_SPEED_HIGH;
                        break;
                case 2:
                        pThis->pRootDevice->speed = USB_SPEED_FULL;
                        break;
                case 3:
                        pThis->pRootDevice->speed = USB_SPEED_LOW;
                        break;
                }
        }

        /* indicate in progress */
        pUrb->status = -EINPROGRESS;
        pUrb->actual_length = 0;
        pUrb->error_count = 0;

        DEBUG_CODE(1,
                   printk(KERN_INFO "%s(%p): nOut=%d, wPacketSize=%d, bAddr=%d, bEnd=%d, pUrb->transfer_buffer=%p\n", \
                          __FUNCTION__, pUrb, nOut, wPacketSize, bAddress, bRemoteEnd, \
                          get_urb_buffer(pUrb)); )

        //if (usb_pipecontrol(nPipe)) wPacketSize = 8; /* for ep0, maxPacketSize is always 8 */  /* no! modified by sunbin*/

        /* remember software state */
        pEnd->dwOffset = 0;
        pEnd->dwRequestSize = 0;
        pEnd->dwIsoPacket = 0;
        pEnd->dwWaitFrame = 0;
        pEnd->bRetries = 0;
        pEnd->wPacketSize = wPacketSize;
        pEnd->bAddress = bAddress;
        pEnd->bEnd = bRemoteEnd;
        pEnd->bTrafficType = (uint8_t)usb_pipetype(nPipe);
        pEnd->bIsTx=(nOut)?TRUE:FALSE;

        /* pEnd->bIsClaimed=(usb_pipeisoc(nPipe) || usb_pipeint(nPipe)) ?0:-1;
         * end must be claimed from my caller
         */

        if (usb_pipecontrol(nPipe)) {
                /* control transfers always start with an OUT */
                nOut = 1;
                pThis->bEnd0Stage = MGC_END0_START;
        }

        /* gather right source of data */
        if (usb_pipeisoc(nPipe)) {
                pBuffer = get_urb_buffer(pUrb) + pUrb->iso_frame_desc[0].offset;
                dwLength = pUrb->iso_frame_desc[0].length;
        } else if (usb_pipecontrol(nPipe)) {
                pBuffer = pUrb->setup_packet;
                dwLength = 8;
        } else {
                pBuffer = get_urb_buffer(pUrb);
                dwLength = pUrb->transfer_buffer_length;
        }

        /* Configure endpoint */
        mgc_hdrc_program_end(pThis, bEnd, pUrb, nOut, pBuffer, dwLength);

        ///* if transmit, start if it is time */
        //if (nOut) {
        //        /* determine if the time is right for a periodic transfer */
        //        if ( usb_pipeisoc(nPipe) || usb_pipeint(nPipe) ) {
        //                DEBUG_CODE(3, printk(KERN_ERR "%s:%d: check whether there's still time for periodic Tx\n",
        //                                     __FUNCTION__, __LINE__ ); );

        //                pEnd->dwIsoPacket = 0;
        //                wFrame = MGC_Read16(pBase, MGC_O_HDRC_FRAME);
        //                if ((pUrb->transfer_flags & USB_ISO_ASAP) ||
        //                                (wFrame >= pUrb->start_frame)) {
        //                        pEnd->dwWaitFrame = 0;
        //                        mgc_hdrc_start_tx(pThis, bEnd);
        //                } else {
        //                        pEnd->dwWaitFrame = pUrb->start_frame;
        //                        /* enable SOF interrupt so we can count down */
        //                        MGC_Write8(pBase, MGC_O_HDRC_INTRUSBE, 0xff);
        //                }
        //        } else {
        //                DEBUG_CODE(3, printk(KERN_ERR "%s:%d: starting Tx\n",
        //                                     __FUNCTION__, __LINE__ ); );
        //                mgc_hdrc_start_tx(pThis, bEnd);
        //        }
        //}
}

/*
 * Submit an URB, either to the virtual root hut or to a real device;
 * it also checks the URB to make sure it's valid.
 * This is called by the Linux "USB core."  mgc_submit_urb locks pThis
 * and the End to use, so make sure the caller releases its locks.
 *
 * also set the hcpriv menber to the localEnd

 * @param pUrb URB pointer (urb = USB request block data structure)
 * @return status code (0 succes)
 */
static int mgc_submit_urb(struct urb* pUrb, int iMemFlags)
{
        MGC_LinuxCd* pThis;
        int nEnd;
        int bustime;
        MGC_LinuxLocalEnd* pEnd;
#if MGC_DEBUG > 0
        MGC_DeviceRequest* pRequest;
#endif
	 uint8_t bCheckBandwidth = FALSE;
        unsigned int pipe = pUrb ? pUrb->pipe : 0;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
#ifdef MGC_PARANOID
        if (!pUrb || !pUrb->dev || !pUrb->dev->bus || !pUrb->dev->bus->hcpriv) {
                DEBUG_CODE(1, printk(KERN_ERR "invalid URB\n"); )
                DEBUG_CODE(3, if ( !pUrb ) {
                printk(KERN_ERR "pUrb is null\n");
                } \
                else if ( !pUrb->dev ) {
                printk(KERN_ERR "pUrb->dev is null\n");
                } \
                else if ( !pUrb->dev->bus ) {
                \
                printk(KERN_ERR "pUrb->dev->bus is null\n");
                } \
                else if ( !pUrb->dev->bus->hcpriv ) {
                \
                printk(KERN_ERR "null instance ptr\n");
                        \
                }
                ; );
                return -EINVAL;
        }

        if ( USB_ENDPOINT_HALTED(pUrb->dev, usb_pipeendpoint (pipe), usb_pipeout (pipe)) ) {
                DEBUG_CODE(1, printk(KERN_ERR "endpoint_halted\n"));
                return -EPIPE;
        }
#endif

        pThis = (MGC_LinuxCd*)pUrb->dev->bus->hcpriv;

#ifdef MGC_PARANOID
        if (mgc_is_corrupt(pThis)) {
                printk(KERN_INFO "corrupted stopping before submit\n");
                mgc_hdrc_stop(pThis);
                MGC_ERR_MODE(pThis);
                return -ENOENT;
        }
#endif

        if ( MGC_IS_ERR(pThis) ) {
                return -ENODEV;
        }

        /* if it is a request to the virtual root hub, delegate */
        if ( !pUrb->dev->parent ) {
                DEBUG_CODE(3, printk(KERN_ERR "%s: delegating to root hub\n", __FUNCTION__); )
                return MGC_VirtualHubSubmitUrb(&(pThis->RootHub), pUrb);
        }

        /* if we are not connected, error */
        if ( !pThis->bIsHost ) {
                DEBUG_CODE(3, printk(KERN_ERR "%s: controller not connected\n", __FUNCTION__); )
                return -ENODEV;
        }

        /* if no root device, assume this must be it */
        if (!pThis->pRootDevice) {
                pThis->pRootDevice = pUrb->dev;
        }

        /* some drivers try to confuse us by linking periodic URBs BOTH ways */
        if (!pUrb->bandwidth && (usb_pipeisoc(pipe) || usb_pipeint(pipe))) {
			{
				bCheckBandwidth = TRUE;
        	}
        }

        /* find appropriate local endpoint to do it */
        nEnd = mgc_find_end(pThis, pUrb);
        if (nEnd < 0) nEnd=1;

        DEBUG_CODE(1, \
                   printk(KERN_INFO "%s: end %d claimed for proto=%d, addr=%d, end=%d\n", \
                          __FUNCTION__, nEnd, usb_pipetype(pipe), \
                          usb_pipedevice(pipe),usb_pipeendpoint(pipe)); )

        /* check bandwidth if needed */
        if (bCheckBandwidth) {
                bustime = usb_check_bandwidth(pUrb->dev, pUrb);
                if (bustime < 0) {
                        return bustime;
                }

                usb_claim_bandwidth(pUrb->dev, pUrb, bustime,
                                    usb_pipeisoc(pipe) ? 1 : 0);
        }

        /* increment urb's reference count, we now control it. */
        pUrb = usb_get_urb(pUrb);

        /* queue or start the URB */
        DEBUG_CODE(2, printk(KERN_INFO "%s: pUrb=%lx, end=%d, bufsize=%x\n", \
                             __FUNCTION__, (unsigned long)pUrb, nEnd, \
                             pUrb->transfer_buffer_length); )


        DEBUG_CODE(2, if (!nEnd) {
        \
        pRequest = (MGC_DeviceRequest*)pUrb->setup_packet;
                \
                printk(KERN_INFO "%s: bmRequestType=%02x, bRequest=%02x, wLength=%04x\n", \
                       __FUNCTION__, pRequest->bmRequestType, pRequest->bRequest, \
                       le16_to_cpu(pRequest->wLength));
                \
        } )


        /* queue URBs */
        pEnd = &(pThis->aLocalEnd[nEnd]);
        spin_lock(&pEnd->Lock);
        pEnd->bIsClaimed=(usb_pipeisoc( pUrb->pipe ) ||
                          usb_pipeint( pUrb->pipe )) ?TRUE:FALSE;

        pUrb->hcpriv = pEnd; /* assign the urb to the endpoint */
        list_add_tail(&pUrb->urb_list, &pEnd->urb_list);
        DEBUG_CODE(1, printk(KERN_INFO "%s: queued URB %lx for end %d\n", \
                             __FUNCTION__, (unsigned long)pUrb, nEnd); )

        if (!pEnd->pUrb) {
                mgc_start_next_urb(pThis, (uint8_t)nEnd);
        }

        spin_unlock(&pEnd->Lock);

#ifdef MGC_PARANOID
        if ( mgc_is_corrupt(pThis) ) {
                printk(KERN_INFO "%s:%d:stopping after submit\n",
                       __FUNCTION__, __LINE__);
                mgc_hdrc_stop(pThis);
                MGC_ERR_MODE(pThis);
                return -ENOENT;
        }
#endif

        DEBUG_CODE(5, dump_urb(pUrb); )
        return 0;

}


/**
 * Cancel URB.
 * @param pUrb URB pointer
 */
static int mgc_unlink_urb(struct urb* pUrb, int status)
{
        uint8_t bOurs;
        int pipe;
        MGC_LinuxCd* pThis;
        MGC_LinuxLocalEnd* pEnd;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s(%lx)\n", \
                             __FUNCTION__, (unsigned long)pUrb); )

        /* sanity */
        if (!pUrb || !pUrb->hcpriv) {
                return -EINVAL;
        }

        if (!pUrb->dev || !pUrb->dev->bus) {
                return -ENODEV;
        }

        pThis = (MGC_LinuxCd*)pUrb->dev->bus->hcpriv;
        if (!pThis) {
                return -ENODEV;
        }

#ifdef MGC_PARANOID
        if (mgc_is_corrupt(pThis)) {
                printk(KERN_INFO "stopping before unlink\n");
                mgc_hdrc_stop(pThis);
                MGC_ERR_MODE(pThis);
                return -EINVAL;
        }
#endif

        /* if it is a request to the virtual root hub, delegate */
        //if (usb_pipedevice (pUrb->pipe) == pThis->RootHub.bAddress) {
        if ( (usb_pipedevice (pUrb->pipe) == pThis->RootHub.bAddress) && (!pUrb->dev->parent)) {
                return MGC_VirtualHubUnlinkUrb(&(pThis->RootHub), pUrb);
        }

        /* stop local end if valid */
        pEnd = (MGC_LinuxLocalEnd*)pUrb->hcpriv;
        if ((pEnd < &(pThis->aLocalEnd[0])) ||
                        (pEnd > &(pThis->aLocalEnd[MGC_C_NUM_EPS-1]))) {
                /* somehow, we got passed a dangling URB pointer */
                return -EINVAL;
        }

        pipe = pUrb->pipe;

        spin_lock(&pEnd->Lock);
        /* delete from list if it was ever added to one */
        bOurs = (pUrb != pEnd->pUrb) && (pUrb->status == -EINPROGRESS);
        if (pUrb->urb_list.next && bOurs) {
                list_del(&pUrb->urb_list);
        }

        if (usb_pipeisoc(pipe) || usb_pipeint(pipe)) {
                /* periodic: shutdown end and any claim */
                mgc_hdrc_stop_end(pThis, pEnd->bLocalEnd);
                pEnd->bIsClaimed = FALSE;
                pEnd->pUrb = NULL;
        } else {
                /* start next URB that might be queued for this end */
                mgc_start_next_urb(pThis, pEnd->bLocalEnd);
        }

        pUrb->hcpriv = NULL;
        spin_unlock(&pEnd->Lock);

        /* release bandwidth if necessary */
        if (pUrb->bandwidth && (usb_pipeisoc(pipe) || usb_pipeint(pipe))) {
                usb_release_bandwidth(pUrb->dev, pUrb,
                                      usb_pipeisoc(pipe) ? 1 : 0);
        }

        pUrb->status = status;
        if (pUrb->complete) {
                pUrb->actual_length = 0;

                DEBUG_CODE(1, \
                           printk(KERN_INFO "%s: completing URB %lx due to unlink\n", \
                                  __FUNCTION__, (unsigned long)pUrb); )

                /* Structure gets somehow corrupted... */
                pUrb->hcpriv=NULL;
				printk("SICTI... mgc_unlink_urb 1\n");
                pUrb->complete(pUrb, NULL);

                DEBUG_CODE(1, \
                           printk(KERN_INFO "%s: URB %lx completed\n", \
                                  __FUNCTION__, (unsigned long)pUrb); )

        } else {
                pUrb->status = -ENOENT;
        }

        bOurs = (pUrb != pEnd->pUrb) && (pUrb->status == -EINPROGRESS);
        if (bOurs) usb_put_urb(pUrb);

#ifdef MGC_PARANOID
        if (mgc_is_corrupt(pThis)) {
                printk(KERN_INFO "stopping after unlink\n");
                mgc_hdrc_stop(pThis);
                MGC_ERR_MODE(pThis);
                return -EINVAL;
        }
#endif

        DEBUG_CODE(2, printk(KERN_INFO "%s => 0\n", __FUNCTION__); )

        return -EINPROGRESS;
}

/**
 * Get the current frame number
 * @param usb_dev pointer to USB device
 * @return frame number
 */
static int mgc_get_frame(struct usb_device* pDevice)
{
        MGC_LinuxCd* pThis = (MGC_LinuxCd*)pDevice->bus->hcpriv;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

        return (int)MGC_Read16(pBase, MGC_O_HDRC_FRAME);
}

/**
 * Service the default endpoint (ep0) as host.
 *
 * @param pThis this
 * @param wCount current byte count in FIFO
 * @param pUrb URB pointer for EP0
 * @return 0 if more packets are required for this transaction
 */
static uint8_t mgc_hdrc_service_host_default(MGC_LinuxCd* pThis,
                uint16_t wCount,
                struct urb* pUrb)
{
        uint8_t bMore = FALSE;
        uint8_t* pFifoDest = NULL;
        uint16_t wFifoCount = 0;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;
        MGC_LinuxLocalEnd* pEnd = &(pThis->aLocalEnd[0]);
        MGC_DeviceRequest* pRequest = (MGC_DeviceRequest*)pUrb->setup_packet;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s(pEnd->wPacketSize = %x, wCount=%04x, pUrb=%lx, bStage=%02x)\n",  \
                             __FUNCTION__, pEnd->wPacketSize, wCount, (unsigned long)pUrb, pThis->bEnd0Stage); )

        if (MGC_END0_IN == pThis->bEnd0Stage) {
                /* we are receiving from peripheral */
                pFifoDest = pUrb->transfer_buffer + pUrb->actual_length;
                wFifoCount = (uint16_t)min(wCount,
                                           (uint16_t)(pUrb->transfer_buffer_length - pUrb->actual_length));

                DEBUG_CODE(3, printk(KERN_INFO "%s:%d: Receiving %d bytes in &%p[%d] (pUrb->actual_length=%u)\n", \
                                     __FUNCTION__, __LINE__, wFifoCount, pUrb->transfer_buffer, \
                                     (unsigned int)pUrb->actual_length, pUrb->actual_length ); )

                mgc_hdrc_unload_fifo(pBase, 0, wFifoCount, pFifoDest);

                pUrb->actual_length += wFifoCount;
                if ((pUrb->actual_length < pUrb->transfer_buffer_length) &&
                                (wCount == pEnd->wPacketSize)) {
                        bMore = TRUE;
                }
        } else {
                /* we are sending to peripheral */
                if ((MGC_END0_START == pThis->bEnd0Stage) &&
                                (pRequest->bmRequestType & USB_DIR_IN)) {
                        DEBUG_CODE(3, printk(KERN_INFO "%s:%d: just did setup, switching to IN\n", \
                                             __FUNCTION__, __LINE__); )

                        /* this means we just did setup; switch to IN */
                        pThis->bEnd0Stage = MGC_END0_IN;
                        bMore = TRUE;
                } else if (pRequest->wLength && ( (MGC_END0_START == pThis->bEnd0Stage) || (MGC_END0_OUT == pThis->bEnd0Stage) ) ) {

                        pThis->bEnd0Stage = MGC_END0_OUT;

							if (pUrb->actual_length < pUrb->transfer_buffer_length) {
                                bMore = TRUE;
                        }else{
									return bMore;
                        	}

                        pFifoDest = (uint8_t*)(pUrb->transfer_buffer + pUrb->actual_length);
                        wFifoCount = (uint16_t)min(pEnd->wPacketSize,
                                                   (uint16_t)(pUrb->transfer_buffer_length - pUrb->actual_length));

                        DEBUG_CODE(3, printk(KERN_INFO "%s:%d:Sending %d bytes to %p\n",  \
                                             __FUNCTION__, __LINE__, wFifoCount, pFifoDest); )

                        mgc_hdrc_load_fifo(pBase, 0, wFifoCount, pFifoDest);

                        pEnd->dwRequestSize = wFifoCount;
                        pUrb->actual_length += wFifoCount;
                }
        }

        return bMore;
}

/**
 * Handle default endpoint interrupt as host.
 * @param pThis this
 */
static void mgc_hdrc_service_default_end(MGC_LinuxCd* pThis)
{
        uint16_t wCsrVal, wCount;
        uint8_t bVal;
        struct urb* pUrb;
        int status = USB_ST_NOERROR;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;
        MGC_LinuxLocalEnd* pEnd = &(pThis->aLocalEnd[0]);
        uint8_t bOutVal = 0;
        uint8_t bComplete = FALSE;
        uint8_t bError = FALSE;
        unsigned long flags;
        spin_lock(&pEnd->Lock);
        pUrb = pEnd->pUrb;
        spin_unlock(&pEnd->Lock);

        spin_lock_irqsave(&pThis->Lock, flags);
        MGC_SelectEnd(pBase, 0);
        wCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_CSR0, 0);
        wCount = MGC_ReadCsr8(pBase, MGC_O_HDRC_COUNT0, 0);
        bVal = (uint8_t)wCsrVal;

        /* check URB */
#ifdef MGC_PARANOID
        if ( pUrb && (pUrb->hcpriv != pEnd)) {
                \
                printk(KERN_ERR "corrupt URB %lx!\n", (unsigned long)pUrb);
                pUrb = NULL;
        }
#endif

        DEBUG_CODE(2, printk(KERN_INFO "CSR0=%04x, wCount=%04x\n", wCsrVal, wCount); )

        /* if we just did status stage, we are done */
        if (MGC_END0_STATUS == pThis->bEnd0Stage) {
                bComplete = TRUE;
        }

        /* prepare status */
        if ((MGC_END0_START == pThis->bEnd0Stage) && !wCount &&
                        (wCsrVal & MGC_M_CSR0_RXPKTRDY)) {

                /* just started and got Rx with no data, so probably missed data */
                status = USB_ST_SHORT_PACKET;
                bError = TRUE;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_FLUSHFIFO);
                MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_FLUSHFIFO);
        }

        if (bVal & MGC_M_CSR0_H_RXSTALL) {
		printk("CSR0_H_RXSTALL: return -EPIPE\n");
                status = USB_ST_STALL;
                bError = TRUE;
        } else if (bVal & MGC_M_CSR0_H_ERROR) {
                //DEBUG_CODE(3, printk(KERN_ERR "no response (error)\n"); mgc_hdrc_dump_regs(pThis, 0); );

                status = USB_ST_NORESPONSE;
                bError = TRUE;
        } else if (bVal & MGC_M_CSR0_H_NAKTIMEOUT) {
                DEBUG_CODE(3, printk(KERN_ERR "NAK timeout pEnd->bRetries=%d\n", pEnd->bRetries); )

                if ( ++pEnd->bRetries < MGC_MAX_RETRIES) {
                        /* cover it up if retries not exhausted */
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 0);
                } else {
                        //DEBUG_CODE(1, printk(KERN_ERR "no response (NAK timeout)\n"); mgc_hdrc_dump_regs(pThis, 0); );

                        status = USB_ST_NORESPONSE;
                        bError = TRUE;
                }
        }

        if (USB_ST_NORESPONSE == status) {

                /* use the proper sequence to abort the transfer */
                if (bVal & MGC_M_CSR0_H_REQPKT) {
                        bVal &= ~MGC_M_CSR0_H_REQPKT;
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bVal);
                        bVal &= ~MGC_M_CSR0_H_NAKTIMEOUT;
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bVal);
                } else {
                        bVal |= MGC_M_CSR0_FLUSHFIFO;
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bVal);
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bVal);
                        bVal &= ~MGC_M_CSR0_H_NAKTIMEOUT;
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bVal);
                }
                MGC_WriteCsr8(pBase, MGC_O_HDRC_NAKLIMIT0, 0, 0);
        }

        if (bError) {
                /* clear it */
                MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 0);
        }

        if (!pUrb) {
                /* stop endpoint since we have no place for its data */

                MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_FLUSHFIFO);
                MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_FLUSHFIFO);
                MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 0);

                /* start next URB that might be queued for it */
                spin_lock(&pEnd->Lock);
                mgc_start_next_urb(pThis, 0);
                spin_unlock(&pEnd->Lock);
                spin_unlock_irqrestore(&pThis->Lock, flags);
                return;
        }

        if (!bComplete && !bError) {
                /* call common logic and prepare response */
                if ( mgc_hdrc_service_host_default(pThis, wCount, pUrb) ) {
                        /* more packets required */
                        bOutVal = (MGC_END0_IN == pThis->bEnd0Stage) ?
                                  MGC_M_CSR0_H_REQPKT : MGC_M_CSR0_TXPKTRDY;

                        DEBUG_CODE(3, printk(KERN_INFO "Need more bytes bOutVal=%04x\n", bOutVal); )
                } else {
                        /* data transfer complete; perform status phase */
                        bOutVal = MGC_M_CSR0_H_STATUSPKT |
                                  (usb_pipeout(pUrb->pipe) ? MGC_M_CSR0_H_REQPKT :
                                   MGC_M_CSR0_TXPKTRDY);
                        /* flag status stage */
                        pThis->bEnd0Stage = MGC_END0_STATUS;

                        DEBUG_CODE(3, printk(KERN_INFO "Data transfer complete, status phase bOutVal=%04x\n", \
                                             bOutVal); )
                }
        }

        /* write CSR0 if needed */
        if (bOutVal) {
                MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, bOutVal);
        }


        /* call completion handler if done */
        if (bComplete || bError) {
                pUrb->status = status;

                DEBUG_CODE(2, printk(KERN_INFO "completing cntrl URB %p, status=%d, len=%x\n", \
                                     pUrb, pUrb->status, pUrb->actual_length); )

                spin_unlock_irqrestore(&pThis->Lock, flags);
                if (pUrb->complete) {
                        pUrb->hcpriv=NULL;
                        pUrb->complete(pUrb, NULL);
                }

                /* start next URB if any */
                spin_lock(&pEnd->Lock);
                mgc_start_next_urb(pThis, 0);
                spin_unlock(&pEnd->Lock);
        } else {
                spin_unlock_irqrestore(&pThis->Lock, flags);
        }
}

/**
 * Service a Tx-Available interrupt for the given endpoint
 * @param pThis instance pointer
 * @param bEnd local endpoint
 */
static void mgc_hdrc_service_tx_avail(MGC_LinuxCd* pThis, uint8_t bEnd)
{
        uint16_t wVal;
        uint16_t wLength;
        uint8_t* pBuffer;
        struct urb* pUrb;
        int nPipe;
        uint8_t bDone = FALSE;
        MGC_LinuxLocalEnd* pEnd = &(pThis->aLocalEnd[bEnd]);
        uint32_t status = USB_ST_NOERROR;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;
        unsigned long flags;

       DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
        spin_lock(&pEnd->Lock);
        pUrb = pEnd->pUrb;
        if (!pUrb) {
		printk("Already handled Urb???\n");
                //mgc_start_next_urb(pThis, bEnd);
                spin_unlock(&pEnd->Lock);
                return;
        }

        pBuffer = get_urb_buffer(pUrb);

        spin_lock_irqsave(&pThis->Lock, flags);
        MGC_SelectEnd(pBase, bEnd);
        wVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);

#if MGC_DEBUG > 0
        /* check URB */
        if (pUrb && (pUrb->hcpriv != pEnd)) {
                printk(KERN_INFO "%s: end %d has corrupt URB %lx!\n",
                       __FUNCTION__, bEnd, (unsigned long)pUrb);
                pUrb = NULL;
        }
#endif

        nPipe = pUrb ? pUrb->pipe : 0;
        DEBUG_CODE(2, printk(KERN_INFO "%s: end %d TxCSR=%04x\n", \
                             __FUNCTION__, bEnd, wVal); )

        /* check for errors */
        if (wVal & MGC_M_TXCSR_H_RXSTALL) {
                printk(KERN_INFO "Tx stall instance=%p\n", pThis);
		printk(KERN_INFO "%s: end %d TxCSR=%04x\n", __FUNCTION__, bEnd, wVal);

		/* stall; record URB status */
                status = USB_ST_STALL;
                /* also record with USB core */
                if (pUrb) {
                        USB_HALT_ENDPOINT(pUrb->dev, usb_pipeendpoint(nPipe),
                                          usb_pipeout(nPipe));
                }
        } else if (wVal & MGC_M_TXCSR_H_ERROR) {
                printk(KERN_INFO "Tx no response\n");

                status = USB_ST_NORESPONSE;
                /* do the proper sequence to abort the transfer */
                wVal &= ~MGC_M_TXCSR_FIFONOTEMPTY;
                wVal |= MGC_M_TXCSR_FLUSHFIFO;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
        } else if (wVal & MGC_M_TXCSR_H_NAKTIMEOUT) {
                status = USB_ST_NORESPONSE;
                if ( pUrb && pUrb->status==-EINPROGRESS ) {
			printk("tx_available: ECONNRESET\n");
                        status = -ECONNRESET;
                }
                /* do the proper sequence to abort the transfer */
                wVal &= ~MGC_M_TXCSR_FIFONOTEMPTY;
                wVal |= MGC_M_TXCSR_FLUSHFIFO;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
                MGC_WriteCsr8(pBase, MGC_O_HDRC_TXINTERVAL, bEnd, 0);
                pEnd->bRetries = 0;
        }
        if (status != USB_ST_NOERROR) {
                printk(KERN_INFO "Tx error=%d\n", status);
                /* reset error bits */
                wVal &= ~(MGC_M_TXCSR_H_ERROR | MGC_M_TXCSR_H_RXSTALL |
                          MGC_M_TXCSR_H_NAKTIMEOUT);
                wVal |= MGC_M_TXCSR_FRCDATATOG;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);
                bDone = TRUE;
        } else if (pUrb) {
                /* see if more transactions are needed */
                pEnd->dwOffset += pEnd->dwRequestSize;
                if (usb_pipeisoc(nPipe)) {
                        /* isoch case */
                        pUrb->iso_frame_desc[pEnd->dwIsoPacket].actual_length =
                                pEnd->dwRequestSize;
                        if (++pEnd->dwIsoPacket >= pUrb->number_of_packets) {
                                bDone = TRUE;
                        } else {
                                /* more to do */
                                pBuffer += pUrb->iso_frame_desc[pEnd->dwIsoPacket].offset;
                                wLength = pUrb->iso_frame_desc[pEnd->dwIsoPacket].length;
                                /* load next packet */
                                mgc_hdrc_load_fifo(pBase, bEnd, wLength, pBuffer);
                                pEnd->dwRequestSize = wLength;
                        }
                } else {
                        /* non-isoch */
                        pBuffer += pEnd->dwOffset;
                        wLength = min((int)pEnd->wPacketSize,
                                      (int)(pUrb->transfer_buffer_length - pEnd->dwOffset));
                        if (pEnd->dwOffset == pUrb->transfer_buffer_length) {
                                /* sent everything; see if we need to send a null */
                                if (!((pEnd->dwRequestSize == pEnd->wPacketSize) &&
                                                (pUrb->transfer_flags & USB_ZERO_PACKET))) {
                                        bDone = TRUE;
                                }
                        } else if (pEnd->dwOffset > pUrb->transfer_buffer_length) {
                                		/* send a null packet over */
                                		bDone = TRUE;
                        				} else {
                                				/* for now, assume any DMA controller can move a maximum-size request */
                                				/* load next packet */
                                				mgc_hdrc_load_fifo(pBase, bEnd, wLength, pBuffer);
                                				pEnd->dwRequestSize = wLength;
                        					}
                }
        } else {
                /* no URB; proceed to next */
                mgc_start_next_urb(pThis, bEnd);
                spin_unlock(&pEnd->Lock);
                spin_unlock_irqrestore(&pThis->Lock, flags);
                return;
        }

        if (bDone) {
                /* set status */
                pUrb->status = status;
                pUrb->actual_length = pEnd->dwOffset;

                /* call completion handler */
                if (usb_pipebulk(nPipe)) {
                        /* we re-use bulk, so re-programming required */
                        pEnd->bIsReady = FALSE;
                        /* release claim if borrowed */
                        if ((bEnd != pThis->bBulkTxEnd) &&
                                        (pThis->bBulkTxEnd != pThis->bBulkRxEnd)) {
                                pEnd->bIsClaimed = FALSE;
                        }
                        /* save data toggle */
                        usb_settoggle(pUrb->dev, pEnd->bEnd, 1,
                                      (wVal & MGC_M_TXCSR_H_DATATOGGLE) ? 1 : 0);
                }

                DEBUG_CODE(1, \
                           printk(KERN_INFO "%s: completing Tx URB %lx, status=%d, len=%x\n", \
                                  __FUNCTION__, (unsigned long)pUrb, pUrb->status, \
                                  pUrb->actual_length); )

                spin_unlock(&pEnd->Lock);
                spin_unlock_irqrestore(&pThis->Lock, flags);

				  MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd)&(~MGC_M_TXCSR_MODE));
                if (pUrb->complete) {
                        pUrb->hcpriv=NULL;
                        pUrb->complete(pUrb, NULL);
                }

                /* proceed to next */
                spin_lock(&pEnd->Lock);
                mgc_start_next_urb(pThis, bEnd);
                spin_unlock(&pEnd->Lock);
        } else {
                /* start next transaction */
				  wVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd);
				  wVal &= ~(MGC_M_TXCSR_DMAENAB | MGC_M_TXCSR_DMAMODE);
				  wVal |= MGC_M_TXCSR_TXPKTRDY | MGC_M_TXCSR_MODE;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd, wVal);

                spin_unlock(&pEnd->Lock);
                spin_unlock_irqrestore(&pThis->Lock, flags);
        }
        DEBUG_CODE(3, printk(KERN_INFO "%s:%d: done\n", \
                             __FUNCTION__, __LINE__); )
}

/**
 * Service an Rx-Ready interrupt for the given endpoint
 * @param pThis instance pointer
 * @param bEnd local endpoint
 */
static void mgc_hdrc_service_rx_ready(MGC_LinuxCd* pThis, uint8_t bEnd)
{
        uint16_t wVal;
        struct urb* pUrb;
        int nPipe;
        unsigned long flags;
        uint8_t bIsochError = FALSE;
        uint8_t bDone = FALSE;
        MGC_LinuxLocalEnd* pEnd = &(pThis->aLocalEnd[bEnd]);
        uint32_t status = USB_ST_NOERROR;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;

        DEBUG_CODE(2,printk(KERN_INFO "=> %s\n", __FUNCTION__);)

        spin_lock(&pEnd->Lock);
        pUrb = pEnd->pUrb;
        nPipe = pUrb ? pUrb->pipe : 0;
        spin_unlock(&pEnd->Lock);

#ifdef MGC_PARANOID
        /* check URB */
        if (pUrb && (pUrb->hcpriv != pEnd) ) {
                printk(KERN_INFO "%s: end %d has corrupt URB %lx (hcpriv=%lx)!\n",
                       __FUNCTION__, bEnd, (unsigned long)pUrb,
                       (unsigned long)pUrb->hcpriv);
                pUrb = NULL;
        }
#endif

        spin_lock_irqsave(&pThis->Lock, flags);
        MGC_SelectEnd(pBase, bEnd);
        wVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd);

        DEBUG_CODE(2, printk(KERN_INFO "%s: end %d RxCSR=%04x\n", \
                             __FUNCTION__, bEnd, wVal); )

        /* check for errors */
        if (wVal & MGC_M_RXCSR_H_RXSTALL) {
                /* stall; record URB status */
		printk("RXCSR_H_RXSTALL: return -EPIPE\n");
                status = USB_ST_STALL;
                /* also record with USB core */
                if (pUrb) {
                        USB_HALT_ENDPOINT(pUrb->dev,
                                          usb_pipeendpoint(nPipe),
                                          usb_pipeout(nPipe));
                }
        } else if (wVal & MGC_M_RXCSR_H_ERROR) {

                DEBUG_CODE(1, printk(KERN_INFO "%s: end %d Rx error\n", \
                                     __FUNCTION__, bEnd); )
                //DEBUG_CODE(1, mgc_hdrc_dump_regs(pThis, bEnd); )
				printk("%s: end %d Rx error  -> MGC_M_RXCSR_H_ERROR\n",  __FUNCTION__, bEnd);
                status = -ECONNRESET;
                /* do the proper sequence to abort the transfer */
                wVal &= ~MGC_M_RXCSR_H_REQPKT;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
                MGC_WriteCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd, 0);
        } else if (wVal & MGC_M_RXCSR_DATAERROR) {
                DEBUG_CODE(1, printk(KERN_INFO "%s: end %d Rx data error\n", \
                                     __FUNCTION__, bEnd); )
                //DEBUG_CODE(1, mgc_hdrc_dump_regs(pThis, bEnd); )

                if (PIPE_BULK == pEnd->bTrafficType) {

                        if ( pUrb && pUrb->status==-EINPROGRESS ) {
									printk("%s: end %d Rx data error  -> MGC_M_RXCSR_DATAERROR\n",  __FUNCTION__, bEnd);
                                status=-ECONNRESET;
                        }

                        /* do the proper sequence to abort the transfer */
                        wVal &= ~MGC_M_RXCSR_H_REQPKT;
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
                        MGC_WriteCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd, 0);
                        pEnd->bRetries=0;
                } else if (PIPE_ISOCHRONOUS == pEnd->bTrafficType) {
                        bIsochError = TRUE;
                }
        }
        /* if no errors, be sure a packet is ready for unloading */
        if (!(wVal & MGC_M_RXCSR_RXPKTRDY) && !status) {
                status = USB_ST_INTERNALERROR;
                DEBUG_CODE(1, \
                           printk(KERN_INFO "%s: Rx interrupt with no errors or packet!\n", \
                                  __FUNCTION__); )

                /* do the proper sequence to abort the transfer */
                wVal &= ~MGC_M_RXCSR_H_REQPKT;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
        }
        if (status != USB_ST_NOERROR) {
                     /* reset error bits */
                DEBUG_CODE(1, printk(KERN_ERR "%s: end %d Rx error, status=%d\n", \
                                     __FUNCTION__, bEnd, status); )
                //DEBUG_CODE(1, mgc_hdrc_dump_regs(pThis, bEnd); )

                wVal &= ~(MGC_M_RXCSR_H_ERROR | MGC_M_RXCSR_DATAERROR |
                          MGC_M_RXCSR_H_RXSTALL | MGC_M_RXCSR_RXPKTRDY);
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
                bDone = TRUE;
        } else {
                if (pUrb) {

				  if(pEnd->pDmaChannel)
				    {
					if(/*MUSB_DMA_STATUS_FREE */1== pThis->pDmaController->pfDmaGetChannelStatus(
					pEnd->pDmaChannel)) {

					    pEnd->dwOffset += pEnd->pDmaChannel->dwActualLength;
					    pUrb->actual_length += pEnd->pDmaChannel->dwActualLength;

					}

					bDone=TRUE;
				    }
			 else
			 	{
                        		bDone = mgc_packet_rx(pThis, bEnd, bIsochError);
			 	}

                } else {
                        /* stop endpoint since we have no place for its data */
                        DEBUG_CODE(1, printk(KERN_ERR "%s: no URB on end %d Rx!\n", \
                                             __FUNCTION__, bEnd); )

                        wVal |= MGC_M_RXCSR_FLUSHFIFO;
                        wVal &= ~MGC_M_RXCSR_H_REQPKT;
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);
                        wVal &= ~(MGC_M_RXCSR_FLUSHFIFO | MGC_M_RXCSR_RXPKTRDY);
                        MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);

                        /* start next URB that might be queued for it */
                        spin_lock(&pEnd->Lock);
                        mgc_start_next_urb(pThis, bEnd);
                        spin_unlock(&pEnd->Lock);
                        spin_unlock_irqrestore(&pThis->Lock, flags);
                        return;
                }
        }

        if (bDone) {
                /* save data toggle if re-using */
                if (usb_pipebulk(nPipe)) {
                        /* we re-use bulk, so re-programming required */
                        pEnd->bIsReady = FALSE;
                        /* release claim if borrowed */
                        if ((bEnd != pThis->bBulkRxEnd) &&
                                        (pThis->bBulkTxEnd != pThis->bBulkRxEnd)) {
                                pEnd->bIsClaimed = FALSE;
                        }
                        /* save data toggle */
                        usb_settoggle(pUrb->dev, pEnd->bEnd, 0,
                                      (wVal & MGC_M_RXCSR_H_DATATOGGLE) ? 1 : 0);


                }

                spin_unlock_irqrestore(&pThis->Lock, flags);

                if (pUrb) {
                        /* set status */
                        pUrb->status = status;

                        DEBUG_CODE(1, printk(KERN_INFO "%s: completing Rx URB %lx, end=%d, status=%d, len=%x\n", \
                                             __FUNCTION__, (unsigned long)pUrb, bEnd, status, \
                                             pUrb->actual_length); )

                        if (pUrb->complete) {
                                pUrb->hcpriv=NULL;
			////	printk("LAND>CallBack called\n");
				pUrb->complete(pUrb, NULL);
                        }
                }

                spin_lock(&pEnd->Lock);
                mgc_start_next_urb(pThis, bEnd);
                spin_unlock(&pEnd->Lock);
        } else {
                /* continue by clearing RxPktRdy and setting ReqPkt */
                wVal &= ~MGC_M_RXCSR_RXPKTRDY;
                wVal |= MGC_M_RXCSR_H_REQPKT;
                MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd, wVal);

                spin_unlock_irqrestore(&pThis->Lock, flags);
        }
}

/**
  * Dump core registers whose reads are non-destructive.
  */
  /*
 static void mgc_hdrc_dump_regs(MGC_LinuxCd* pThis, uint8_t bEnd)
 {
         uint8_t* pBase = (uint8_t*)pThis->pRegs;

         MGC_SelectEnd(pBase, bEnd);

         if (!bEnd) {
                 printk(KERN_INFO " 0: CSR0=%04x, Count0=%02x, Type0=%02x, NAKlimit0=%02x\n",
                        MGC_ReadCsr16(pBase, MGC_O_HDRC_CSR0, 0),
                        MGC_ReadCsr8(pBase, MGC_O_HDRC_COUNT0, 0),
                        MGC_ReadCsr8(pBase, MGC_O_HDRC_TYPE0, 0),
                        MGC_ReadCsr8(pBase, MGC_O_HDRC_NAKLIMIT0, 0));
         } else {
                 printk(KERN_INFO "%2d: TxCSR=%04x, TxMaxP=%04x, TxType=%02x, TxInterval=%02x\n",
                        bEnd,
                        MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, bEnd),
                        MGC_ReadCsr16(pBase, MGC_O_HDRC_TXMAXP, bEnd),
                        MGC_ReadCsr8(pBase, MGC_O_HDRC_TXTYPE, bEnd),
                        MGC_ReadCsr8(pBase, MGC_O_HDRC_TXINTERVAL, bEnd));
                 printk(KERN_INFO "    RxCSR=%04x, RxMaxP=%04x, RxType=%02x, RxInterval=%02x, RxCount=%04x\n",
                        MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, bEnd),
                        MGC_ReadCsr16(pBase, MGC_O_HDRC_RXMAXP, bEnd),
                        MGC_ReadCsr8(pBase, MGC_O_HDRC_RXTYPE, bEnd),
                        MGC_ReadCsr8(pBase, MGC_O_HDRC_RXINTERVAL, bEnd),
                        MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCOUNT, bEnd));
         }

         if ( pThis->bIsHost && pThis->bIsMultipoint) {
                 printk(KERN_INFO "    TxAddr=%02x, TxHubAddr=%02x, TxHubPort=%02x\n",
                        MGC_Read8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_TXFUNCADDR)),
                        MGC_Read8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_TXHUBADDR)),
                        MGC_Read8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_TXHUBPORT)));
                printk(KERN_INFO "    RxAddr=%02x, RxHubAddr=%02x, RxHubPort=%02x\n",
                        MGC_Read8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_RXFUNCADDR)),
                        MGC_Read8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_RXHUBADDR)),
                       MGC_Read8(pBase, MGC_BUSCTL_OFFSET(bEnd, MGC_O_HDRC_RXHUBPORT)));
        }
 }
 */

