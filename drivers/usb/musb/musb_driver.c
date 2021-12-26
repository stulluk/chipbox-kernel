/*****************************************************************
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
 * Inventra Controller Driver (ICD) for Linux.
 * This consists of a Host Controller Driver (HCD) and optionally,
 * a Gadget provider.
 * $Revision: 1.1.1.1 $
 */

/**
 * Introduction.
 * The ICD works like the other Linux HCDs/Gadgets: it is threadless,
 * so it does everything either in response to an interrupt,
 * or during a call from an upper layer.
 * It implements a virtual root hub, so as to make uniform use
 * of the Linux hub driver.
 *
 * The Linux (host-side) USB core has no concept of binding.
 * Instead, it encodes target address and protocol information in an int
 * member of "URBs" (USB Request Blocks), which may be submitted at any time.
 * This means that if an HCD cannot fulfill a request (insufficient
 * available bus bandwidth or a temporary lack of resources), it must
 * return an error, and this can happen at any time during the lifetime
 * of a "session" with a device.
 *
 * For the HDRC, local endpoint 0 is the only choice for control traffic,
 * so it is reprogrammed as needed, and locked during transfers.
 * Bulk transfers are queued to the available local endpoint with
 * the smallest possible FIFO in the given direction
 * that will accomodate the transactions.
 *
 * A typical response to the completion of a periodic URB is immediate
 * submission of another one, so the HCD does not assume it can reprogram
 * a local periodic-targetted endpoint for another purpose.
 * Instead, submission of a periodic URB is taken as a permanent situation,
 * so that endpoint is untouched.  This is again due to lack of binding.
 *
 * One could imagine reprogramming periodic endpoints for other uses
 * between their polling intervals, effectively interleaving traffic on them.
 * Unfortunately, this assumes no device would ever NAK periodic tokens.
 * This is because the core does not notify software when an attempted
 * periodic transaction is NAKed (its NAKlimit feature is only for
 * control/bulk).
 */

/*
 * Optional macros:
 *
 * MGC_FLAT_REG	if defined, use the core's flag register model
 *
 * MGC_DEBUG		0 => absolutely no diagnostics
 * 			1 => minimal diagnostics (basic operational states)
 * 			2 => 1 + detailed debugging of interface with USB core
 * 			3 => 2 + internal debugging (e.g. every register write)
 * 			4 => 3 + shared-IRQ-related checking
 *
 * Options taken from linux/config.h:
 *
 * CONFIG_PROC_FS	enables statistics/state info in /proc/musbhdrc<n>
 * 			where 0 <= n < number of instances of driver
 *
 * CONFIG_PM		enables PCI power-management
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/smp_lock.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/proc_fs.h>

#include <linux/mem_define.h>

#include <asm/uaccess.h>
#include <asm/io.h>
//#include <musbdefs.h>
#ifdef CONFIG_USB_DEBUG
#define DEBUG
#else
#undef DEBUG
#endif

#include <linux/usb.h>
#include <linux/version.h>

#include "musbdefs.h"
/* Pick up definitions of externs declared above */
#include "board.h"

#ifdef MGC_DMA
#include "dma.h"
#include "musbhsdma.c"
#endif

static u64 dma_mask = DMA_32BIT_MASK;
static char usb_stat[] = { "0\n" };

#define RETURN_IRQ_HANDLED return(IRQ_HANDLED)
#define RETURN_IRQ_NONE return(IRQ_NONE)

/****************************** CONSTANTS ********************************/

#define DRIVER_VERSION "v2.4"
#define DRIVER_AUTHOR "Mentor Graphics Corp."
#define DRIVER_DESC "Inventra Controller Driver"

/** time (millseconds) to wait before a restart */
#define MGC_RESTART_TIME        5000

/** how many babbles to allow before giving up */
#define MGC_MAX_BABBLE_COUNT    10

/* how many buss errors before stopping the operations */
#define MGC_MAX_VBUS_ERRORS	3

/************extern functions in musb_buffer.c************/
extern int musb_buffer_create (MGC_LinuxCd* pThis);
extern void musb_buffer_destroy (MGC_LinuxCd* pThis);
extern void *musb_buffer_alloc (
	struct usb_bus 		*bus,
	size_t			size,
	int			mem_flags,
	dma_addr_t		*dma
);

extern void musb_buffer_free (
	struct usb_bus 		*bus,
	size_t			size,
	void 			*addr,
	dma_addr_t		dma
);
/******************************* FORWARDS ********************************/

/* driver functions */
static MGC_LinuxCd* mgc_controller_init(void* pDevice, uint16_t wType,
                                        int nIrq, void* pRegs, const char* pName);
static irqreturn_t mgc_controller_isr(int irq, void *__hci, struct pt_regs *r);

/* Linux USBD glue */
static int mgc_submit_urb(struct urb* pUrb, int iMemFlags);
static int mgc_unlink_urb(struct urb* pUrb, int status);
// DELME static void mgc_end_disable(struct usb_device* pDevice, int bEndpointAddress);

// DELME static int mgc_alloc_device(struct usb_device* pDevice);
// DELME static int mgc_free_device(struct usb_device* pDevice);
static int mgc_get_frame(struct usb_device* pDevice);

/* virtual hub glue */
static void mgc_set_port_power(void* pPrivateData, uint8_t bPortIndex,
                               uint8_t bPower);
static void mgc_set_port_enable(void* pPrivateData, uint8_t bPortIndex,
                                uint8_t bEnable);
static void mgc_set_port_suspend(void* pPrivateData, uint8_t bPortIndex,
                                 uint8_t bSuspend);
static void mgc_set_port_reset(void* pPrivateData, uint8_t bPortIndex,
                               uint8_t bReset);

/* HCD helpers */
static void mgc_set_timer(MGC_LinuxCd* pThis,
                          void (*pfFunc)(unsigned long),
                          unsigned long pParam, unsigned long millisecs);

/* HDRC functions */
static void mgc_hdrc_reset_off(unsigned long param);
static void mgc_hdrc_drop_resume(unsigned long pParam);
static void mgc_hdrc_restart(unsigned long pParam);
static void mgc_hdrc_service_usb(MGC_LinuxCd* pThis, uint8_t reg);
static void mgc_hdrc_disable(MGC_LinuxCd* pThis);
static uint8_t mgc_hdrc_init(uint16_t wType, MGC_LinuxCd* pThis);

/* HDRC host-mode functions */
static void mgc_hdrc_stop_end(MGC_LinuxCd* pThis, uint8_t bEnd);
static void mgc_hdrc_start_tx(MGC_LinuxCd* pThis, uint8_t bEnd);
static void mgc_hdrc_service_default_end(MGC_LinuxCd* pThis);
static void mgc_hdrc_service_tx_avail(MGC_LinuxCd* pThis, uint8_t bEnd);
static void mgc_hdrc_service_rx_ready(MGC_LinuxCd* pThis, uint8_t bEnd);

static void mgc_setting_ulpi(MGC_LinuxCd* pThis);
static void direct_bus_shutdown(void);
int __init mgc_module_init(void);

/* DMA buffers */
/*static void* mgc_buffer_alloc(struct usb_bus* pBus, size_t nSize,
                              int iMemFlags, dma_addr_t* pDmaAddress);
static void mgc_buffer_free(struct usb_bus* pBus, size_t nSize,
                            void* address, dma_addr_t DmaAddress);*/

/******************************* GLOBALS *********************************/

static int mgc_instances_count=0;
static MGC_LinuxCd** mgc_driver_instances;

static const char mgc_hcd_name[] = "musb-hcd";
static unsigned int mgc_index = 0;

/* By KB KIm 2011.01.25 */
static unsigned char UsbDviceNotConnected = 1;

//static uint8_t mgc_ignore_disconnect = -1;

/* Linux USBD calls these */
static struct usb_operations mgc_ops = {
	.get_frame_number =	mgc_get_frame,
	.submit_urb =		mgc_submit_urb,
	.unlink_urb =		mgc_unlink_urb,
	//.buffer_alloc =		mgc_buffer_alloc,
	//.buffer_free =		mgc_buffer_free
	.buffer_alloc =		musb_buffer_alloc,
	.buffer_free =		musb_buffer_free
};

/**************************************************************************
 * HDRC functions
 **************************************************************************/

/**
 * Timer completion callback to finish resume handling started in ISR
 */
static void mgc_hdrc_drop_resume(unsigned long pParam)
{
        uint8_t power;
        MGC_LinuxCd* pThis = (MGC_LinuxCd*)pParam;
        void* pBase = pThis->pRegs;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

        power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
        MGC_Write8(pBase, MGC_O_HDRC_POWER, power & ~MGC_M_POWER_RESUME);
        MGC_VirtualHubPortResumed(&(pThis->RootHub), 0);
}

/**
 * Timer completion callback to request session again
 * (to avoid self-connecting)
 */
static void mgc_hdrc_restart(unsigned long pParam)
{
        MGC_LinuxCd* pThis = (MGC_LinuxCd*)pParam;

        mgc_hdrc_start(pThis);
}

/**
 * Load an HDRC FIFO
 *
 * @param pBase base address of HDRC
 * @param bEnd local endpoint
 * @param wCount how many bytes to load
 * @param pSource data buffer
 */
static void mgc_hdrc_load_fifo(const uint8_t* pBase, uint8_t bEnd,
                               uint16_t wCount, const uint8_t* pSource)
{
        uint16_t wIndex, wIndex32;
        uint16_t wCount32 = wCount >> 2;
        uint8_t bFifoOffset = MGC_FIFO_OFFSET(bEnd);

        DEBUG_CODE(2, \
                   printk(KERN_INFO "=> %s: pBase=%p, bEnd=%d, wCount=%04x, pSrc=%p\n", \
                          __FUNCTION__, pBase, bEnd, wCount, pSource); )
//	if ( wCount%512==0) { 
//		MGC_Write32(pBase, 0x208, pSource); /* setting DMA addr */
//		MGC_Write32(pBase, 0x20c, wCount); /* setting byte numbers that will be received */
//		MGC_Write16(pBase, 0x204, 0x0001 | 0x0008 | 0x0002|(bEnd<<4) | (3 << 9)); /* setting CTRL regs */
//
//		udelay(20);
//		while(0 == MGC_Read16(pBase, 0x200));
//	}
//else{
	for (wIndex = wIndex32 = 0; wIndex32 < wCount32; wIndex32++, wIndex += 4) {
                MGC_Write32(pBase, bFifoOffset, *((uint32_t*)&(pSource[wIndex])));
        }

        for (; wIndex < wCount; wIndex++) {
                MGC_Write8(pBase, bFifoOffset, pSource[wIndex]);
        }
//	}
#if MGC_DEBUG > 0
        { 
                int i;
                printk("\nLoad = { ");
                for (i=0;i<wIndex;i++) printk("0x%02x, ", pSource[i]);
                printk("}\n");
        }
#endif
}

/**
 * Unload an HDRC FIFO
 *
 * @param pBase base address of HDRC
 * @param bEnd local endpoint
 * @param wCount how many bytes to unload
 * @param pDest data buffer
 */
static void mgc_hdrc_unload_fifo(const uint8_t* pBase, uint8_t bEnd,
                                 uint16_t wCount, uint8_t* pDest)
{
        uint16_t wIndex=0, wIndex32;
        uint16_t wCount32 = wCount >> 2;
        uint8_t bFifoOffset = MGC_FIFO_OFFSET(bEnd);

        DEBUG_CODE(2, \
                   printk(KERN_INFO "=> %s: pBase=%lx, bEnd=%d, wCount=%04x, pDest=%lx\n", \
                          __FUNCTION__, (unsigned long)pBase, bEnd, \
                          wCount, (unsigned long)pDest); )
#if 0
	if ( wCount%512==0) { 
		MGC_Write32(pBase, 0x208, virt_to_phys(pDest)); /* setting DMA addr */
		MGC_Write32(pBase, 0x20c, wCount); /* setting byte numbers that will be received */
		MGC_Write16(pBase, 0x204, 0x0001 | 0x0008 | (bEnd<<4) | (3 << 9)); /* setting CTRL regs */

		udelay(20);
		while(0 == MGC_Read16(pBase, 0x200));
	}
	else {
#endif		
		/* doublewords when possible */
		for (wIndex = wIndex32 = 0; wIndex32 < wCount32; wIndex32++, wIndex += 4) {
			*((uint32_t*)&(pDest[wIndex])) =
				MGC_Read32(pBase, bFifoOffset);
		}

		while (wIndex < wCount) {
			pDest[wIndex++] =
				MGC_Read8(pBase, bFifoOffset);
		}
//	}
#if MGC_DEBUG > 0
	{ 
		int i;
		printk("\nUnload = { ");
		for (i=0;i<wCount;i++) printk("0x%02x, ", pDest[i]);
		printk("};\n");
	}
#endif
}

/**
 * Interrupt Service Routine to record USB "global" interrupts.
 * Since these do not happen often and signify things of
 * paramount importance, it seems OK to check them individually.
 *
 * @param pThis instance pointer
 * @param reg IntrUSB register contents
 */
void mgc_hdrc_service_usb(MGC_LinuxCd* pThis, uint8_t reg)
{
        uint8_t bEnd;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;
        uint8_t devctl = MGC_Read8(pBase, MGC_O_HDRC_DEVCTL);
        uint8_t power = MGC_Read8(pBase, MGC_O_HDRC_POWER);
		uint16_t wType;		
			uint16_t wCsr;
			struct urb* pUrb;
			MGC_LinuxLocalEnd* pEnd;

        DEBUG_CODE(2, printk(KERN_INFO "<== Power=%02x, DevCtl=%02x, reg=0x%x\n", power, devctl, reg); )

        if (reg & MGC_M_INTR_RESUME) {
                if (devctl & MGC_M_DEVCTL_HM) {
                        MGC_HST_MODE(pThis);
                        power &= ~MGC_M_POWER_SUSPENDM;
                        MGC_Write8(pBase, MGC_O_HDRC_POWER, power | MGC_M_POWER_RESUME);
                        mgc_set_timer(pThis, mgc_hdrc_drop_resume,
                                      (unsigned long)pThis, 40);
                }
        }

        if (reg & MGC_M_INTR_SESSREQ) {
                MGC_Write8(pBase, MGC_O_HDRC_DEVCTL,
                           MGC_M_DEVCTL_SESSION);
                pThis->bEnd0Stage = MGC_END0_START;
                MGC_HST_MODE(pThis);
        }

        if (reg & MGC_M_INTR_VBUSERROR)
        {
			printk("mgc_hdrc_service_usb : MGC_M_INTR_VBUSERROR\n");
#if 1
			uint8_t regg = MGC_Read8(pBase,  MGC_O_HDRC_DEVCTL);
			regg &= 0xFE;
			MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, regg);
			mdelay(5000);
			regg = MGC_Read8(pBase,  MGC_O_HDRC_DEVCTL);
			regg  |= MGC_M_DEVCTL_SESSION;
			MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, regg);
#else
			free_irq(pThis->nIrq, pThis);
			mgc_hdrc_stop(pThis);
			if (pThis->bIsMultipoint)
			{
				wType = MGC_CONTROLLER_MHDRC;
			}
			else
			{
				wType = MGC_CONTROLLER_HDRC;
			}
			MGC_EXTRA_PRE_INIT(pThis->nIrq, pThis->pRegs);
			if(!mgc_hdrc_init(wType, pThis))
			{
			   	printk("mgc_hdrc_service_usb : MGC_M_INTR_VBUSERROR : error to re init \n");
				return;
			}

			request_irq (pThis->nIrq, mgc_controller_isr, /*SA_SHIRQ|*/SA_INTERRUPT, pThis->aName, pThis);
			mgc_hdrc_start(pThis);
			MGC_EXTRA_POST_INIT(pThis);
#endif
		/*{
                if (devctl & MGC_M_DEVCTL_HM) {
                        MGC_HST_MODE(pThis);
                        MGC_VirtualHubPortDisconnected(&(pThis->RootHub), 0);
                        pThis->pRootDevice = NULL;

                        for (bEnd = 0; bEnd < pThis->bEndCount; bEnd++) {
                                mgc_hdrc_stop_end(pThis, bEnd);
                        }
                }

					usb_stat[0] = 0x30;
        }	*/
		  	   
		/*	
                if ( pThis->bVbusErrors++ > MGC_MAX_VBUS_ERRORS ) {
                        mgc_hdrc_stop(pThis);
                        MGC_VirtualHubPortDisconnected(&(pThis->RootHub), 0);
                        pThis->pRootDevice = NULL;
                        MGC_ERR_MODE(pThis);
                        printk(KERN_ERR "Stopped host due to Vbus error\n");
                }
         */
			
        }

        if (reg & MGC_M_INTR_SUSPEND) {
                if (devctl & MGC_M_DEVCTL_HM) {
                        mgc_hdrc_stop(pThis);
                }
        }

        if (reg & MGC_M_INTR_CONNECT) {
		uint8_t bSpeed = 1;
		uint8_t bHubSpeed = 2;
		// printk("mgc_hdrc_service_usb : MGC_M_INTR_CONNECT\n");

        UsbDviceNotConnected = 0;

			usb_stat[0] = 0x31;
						
			MGC_SelectEnd(pBase, 0);			
        	wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_CSR0, 0);
			wCsr |= MGC_M_CSR0_FLUSHFIFO;
			wCsr &= ~(MGC_M_CSR0_RXPKTRDY | MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_H_RXSTALL | MGC_M_CSR0_H_SETUPPKT | \
								MGC_M_CSR0_H_ERROR | MGC_M_CSR0_H_REQPKT | MGC_M_CSR0_H_STATUSPKT | MGC_M_CSR0_H_NAKTIMEOUT);
			MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsr);
			wCsr &= ~MGC_M_CSR0_FLUSHFIFO;
			MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsr);

			MGC_SelectEnd(pBase, pThis->bBulkTxEnd);			
			wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, 0);
			wCsr |= MGC_M_TXCSR_FLUSHFIFO;
			wCsr &= ~(MGC_M_TXCSR_TXPKTRDY | MGC_M_TXCSR_FIFONOTEMPTY | MGC_M_TXCSR_H_ERROR | MGC_M_TXCSR1_P_SENDSTALL | \
								MGC_M_TXCSR1_H_RXSTALL | MGC_M_TXCSR1_H_NAKTIMEOUT | MGC_M_TXCSR_DMAENAB | MGC_M_TXCSR_AUTOSET);
			MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, 0, wCsr);
			wCsr &= ~MGC_M_TXCSR_FLUSHFIFO;
			MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, 0, wCsr);

			MGC_SelectEnd(pBase, pThis->bBulkRxEnd);
			wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, 0);
			wCsr |= MGC_M_RXCSR_FLUSHFIFO;
			wCsr &= ~(MGC_M_RXCSR_RXPKTRDY | MGC_M_RXCSR_FIFOFULL | MGC_M_RXCSR_H_ERROR | MGC_M_RXCSR_DATAERROR | MGC_M_RXCSR_AUTOCLEAR | \
								MGC_M_RXCSR_H_REQPKT | MGC_M_RXCSR_H_RXSTALL | MGC_M_RXCSR_DISNYET | MGC_M_RXCSR_DMAENAB | MGC_M_RXCSR_AUTOREQ);
			MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, 0, wCsr);
			wCsr &= ~MGC_M_RXCSR_FLUSHFIFO;
			MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, 0, wCsr);

			pEnd = &(pThis->aLocalEnd[pThis->bBulkTxEnd]);
			if( pThis->pDmaController && pEnd->pDmaChannel ){
				pThis->pDmaController->pfDmaReleaseChannel(pEnd->pDmaChannel);
				pEnd->pDmaChannel = NULL;

			}

			if (pEnd->pUrb) {
           	usb_put_urb(pEnd->pUrb);
				pEnd->pUrb = NULL;

          }
			while (!list_empty(&pEnd->urb_list)){

				pUrb = list_entry(pEnd->urb_list.next, struct urb, urb_list);
              if (pUrb){

              	list_del(&pUrb->urb_list);
					pUrb->status = USB_ST_REMOVED;
              	if (pUrb->complete) {

                  	pUrb->hcpriv=NULL;			
						pUrb->complete(pUrb, NULL);
                  }
              }
			}                
			
			if( pThis->bBulkRxEnd != pThis->bBulkTxEnd ){
				pEnd = &(pThis->aLocalEnd[pThis->bBulkRxEnd]);
				if( pThis->pDmaController && pEnd->pDmaChannel ){
					pThis->pDmaController->pfDmaReleaseChannel(pEnd->pDmaChannel);
					pEnd->pDmaChannel = NULL;
				}
				if (pEnd->pUrb) {
           		usb_put_urb(pEnd->pUrb);
					pEnd->pUrb = NULL;
          		}
				while (!list_empty(&pEnd->urb_list)){
					pUrb = list_entry(pEnd->urb_list.next, struct urb, urb_list);
              	if (pUrb){
              		list_del(&pUrb->urb_list);
						pUrb->status = USB_ST_REMOVED;
              		if (pUrb->complete) {
                  		pUrb->hcpriv=NULL;			
							pUrb->complete(pUrb, NULL);
                  	}
              	}
				}
			}
	
                pThis->pRootDevice = NULL;
                //MGC_Write8(pThis->pRegs, MGC_O_HDRC_FADDR, 0);
	
		MGC_HST_MODE(pThis);		
		/* flush endpoints */
		for(bEnd = 0; bEnd < pThis->bEndCount; bEnd++) {
			mgc_hdrc_stop_end(pThis, bEnd);
		}

		if(devctl & MGC_M_DEVCTL_LSDEV) {
			bSpeed = 3;
			bHubSpeed = 0;
		} else if(devctl & MGC_M_DEVCTL_FSDEV) {
			/* NOTE: full-speed is "speculative" until reset */
			bSpeed = 2;
			bHubSpeed = 1;
		}

		pThis->bRootSpeed = bSpeed;
		if(pThis->bIsMultipoint) {
			/* set speed for EP0 */
			MGC_SelectEnd(pBase, 0);
			MGC_WriteCsr8(pBase, MGC_O_HDRC_TYPE0, 0, 
					(bSpeed << 6));
		}

		MGC_VirtualHubPortConnected(&(pThis->RootHub), 0, bHubSpeed);
		// udelay(3000);
		printk("mgc_hdrc_service_usb : MGC_M_INTR_CONNECT\n");
		// mdelay(3000);

		//usb_stat[0] = 0x31;
        }

        if (reg & MGC_M_INTR_DISCONNECT) { // && !mgc_ignore_disconnect) {
        printk("mgc_hdrc_service_usb : MGC_M_INTR_DISCONNECT\n");
        UsbDviceNotConnected = 1;

                //if (devctl & MGC_M_DEVCTL_HM) {
				  if (devctl) {
                        MGC_HST_MODE(pThis);
                        MGC_VirtualHubPortDisconnected(&(pThis->RootHub), 0);
                        pThis->pRootDevice = NULL;

                        /* flush endpoints */
                        for (bEnd = 0; bEnd < pThis->bEndCount; bEnd++) {
                                mgc_hdrc_stop_end(pThis, bEnd);
                        }
                }
						
			MGC_SelectEnd(pBase, 0);
			
        	wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_CSR0, 0);
			wCsr |= MGC_M_CSR0_FLUSHFIFO;
			wCsr &= ~(MGC_M_CSR0_RXPKTRDY | MGC_M_CSR0_TXPKTRDY | MGC_M_CSR0_H_RXSTALL | MGC_M_CSR0_H_SETUPPKT | \
								MGC_M_CSR0_H_ERROR | MGC_M_CSR0_H_REQPKT | MGC_M_CSR0_H_STATUSPKT | MGC_M_CSR0_H_NAKTIMEOUT);
			MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsr);
			wCsr &= ~MGC_M_CSR0_FLUSHFIFO;
			MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsr);

			MGC_SelectEnd(pBase, pThis->bBulkTxEnd);
			wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_TXCSR, 0);
			wCsr |= MGC_M_TXCSR_FLUSHFIFO;
			wCsr &= ~(MGC_M_TXCSR_TXPKTRDY | MGC_M_TXCSR_FIFONOTEMPTY | MGC_M_TXCSR_H_ERROR | MGC_M_TXCSR1_P_SENDSTALL | \
								MGC_M_TXCSR1_H_RXSTALL | MGC_M_TXCSR1_H_NAKTIMEOUT | MGC_M_TXCSR_DMAENAB | MGC_M_TXCSR_AUTOSET);
			MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, 0, wCsr);
			wCsr &= ~MGC_M_TXCSR_FLUSHFIFO;
			MGC_WriteCsr16(pBase, MGC_O_HDRC_TXCSR, 0, wCsr);

			MGC_SelectEnd(pBase, pThis->bBulkRxEnd);
			wCsr = MGC_ReadCsr16(pBase, MGC_O_HDRC_RXCSR, 0);
			wCsr |= MGC_M_RXCSR_FLUSHFIFO;
			wCsr &= ~(MGC_M_RXCSR_RXPKTRDY | MGC_M_RXCSR_FIFOFULL | MGC_M_RXCSR_H_ERROR | MGC_M_RXCSR_DATAERROR | MGC_M_RXCSR_AUTOCLEAR | \
								MGC_M_RXCSR_H_REQPKT | MGC_M_RXCSR_H_RXSTALL | MGC_M_RXCSR_DISNYET | MGC_M_RXCSR_DMAENAB | MGC_M_RXCSR_AUTOREQ);
			MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, 0, wCsr);
			wCsr &= ~MGC_M_RXCSR_FLUSHFIFO;
			MGC_WriteCsr16(pBase, MGC_O_HDRC_RXCSR, 0, wCsr);
	
		usb_stat[0] = 0x30;
        }

        if (reg & MGC_M_INTR_RESET) {
                if (devctl & MGC_M_DEVCTL_HM) {
                        /* we are babbling */
                        printk(KERN_ERR "Stopping host due to babble\n");
                        mgc_hdrc_stop(pThis);
                        MGC_ERR_MODE(pThis);

                        MGC_VirtualHubPortDisconnected(&(pThis->RootHub), 0);
                        pThis->pRootDevice = NULL;

                        /* restart session after cooldown unless threshold reached */
                        if ( pThis->nBabbleCount++ < MGC_MAX_BABBLE_COUNT) {
                                mgc_set_timer(pThis, mgc_hdrc_restart,
                                              (unsigned long)pThis, MGC_RESTART_TIME);
                        }
                } else {
                }
        }
}

/**
 * Program the HDRC to start (enable interrupts, etc.).
 */
void mgc_hdrc_start(MGC_LinuxCd* pThis)
{
        uint8_t bEnd=1;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

        /* hook extra start handling */
        MGC_EXTRA_START(pThis);

        /* TODO: always set ISOUPDATE in POWER (periph mode) and leave it on! */

        /*  Set INT enable registers, enable interrupts */
        MGC_Write16(pBase, MGC_O_HDRC_INTRTXE, pThis->wEndMask);
        MGC_Write16(pBase, MGC_O_HDRC_INTRRXE, pThis->wEndMask & 0xfffe);
        MGC_Write8(pBase, MGC_O_HDRC_INTRUSBE, 0xf7);

        MGC_Write8(pBase, MGC_O_HDRC_TESTMODE, 0);

        /* enable high-speed/low-power and start session */
        MGC_Write8(pBase, MGC_O_HDRC_POWER,
                   MGC_M_POWER_SOFTCONN | MGC_M_POWER_HSENAB);

        /* enable high-speed/low-power and start session & suspend IM host*/
        MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, MGC_M_DEVCTL_SESSION);

        /* init the local ends */
        for (bEnd=1; bEnd < pThis->bEndCount; bEnd++) {
                spin_lock( &pThis->aLocalEnd[bEnd].Lock );
                INIT_LIST_HEAD( &(pThis->aLocalEnd[bEnd].urb_list) );
                pThis->aLocalEnd[bEnd].bIsClaimed=FALSE;
                spin_unlock( &pThis->aLocalEnd[bEnd].Lock );
        }

        /* reset the counters */
        pThis->bVbusErrors=0;
}

/**
 * Disable the HDRC (disable & flush interrupts).
 */
void mgc_hdrc_disable(MGC_LinuxCd* pThis)
{
        uint16_t temp;
        uint8_t* pBase = (uint8_t*)pThis->pRegs;

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

        /* disable interrupts */
        MGC_Write8(pBase, MGC_O_HDRC_INTRUSBE, 0);
        MGC_Write16(pBase, MGC_O_HDRC_INTRTX, 0);
        MGC_Write16(pBase, MGC_O_HDRC_INTRRX, 0);

        /* off */
        MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, 0);

        /*  flush pending interrupts */
        temp = MGC_Read8(pBase, MGC_O_HDRC_INTRUSB);
        temp = MGC_Read16(pBase, MGC_O_HDRC_INTRTX);
        temp = MGC_Read16(pBase, MGC_O_HDRC_INTRRX);

        DEBUG_CODE(3, printk(KERN_INFO "%s: Interrupts disabled\n",
                             __FUNCTION__); )
}

/**
 * Program the HDRC to stop (disable interrupts, etc.).
 */
void mgc_hdrc_stop(MGC_LinuxCd* pThis)
{
        uint8_t bEnd;

        //DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
        printk(KERN_INFO "=> %s\n", __FUNCTION__);

        mgc_hdrc_disable(pThis);

        /* flush endpoints */
        for (bEnd = 0; bEnd < min((uint8_t)16, pThis->bEndCount); bEnd++) {
                mgc_hdrc_stop_end(pThis, bEnd);
        }
}

uint8_t mgc_hdrc_vbusctrl_ulpi(MGC_LinuxCd* pThis, uint8_t bExtSource, uint8_t bExtIndicator)
{
        uint8_t bVal;
        uint8_t* pBase = pThis->pRegs;

        /* ensure not powered down */
        if (MGC_Read8(pBase, MGC_O_HDRC_POWER) & MGC_M_POWER_ENSUSPEND) {
                return FALSE;
        }

        bVal = bExtSource ? MGC_M_ULPI_VBUSCTL_USEEXTVBUS : 0;
        bVal |= bExtIndicator ? MGC_M_ULPI_VBUSCTL_USEEXTVBUSIND : 0;
        MGC_Write8(pBase, MGC_O_HDRC_ULPI_VBUSCTL, bVal);

        return TRUE;
}

uint8_t mgc_hdrc_read_ulpi(MGC_LinuxCd* pThis, uint8_t bAddr, uint8_t* pbData)
{
        uint8_t bCtl = 0, retry = 128;
        uint8_t* pBase = pThis->pRegs;

        /* ensure not powered down */
        if (MGC_Read8(pBase, MGC_O_HDRC_POWER) & MGC_M_POWER_ENSUSPEND) {
                return FALSE;
        }

        /* polled */
        MGC_Write8(pBase, MGC_O_HDRC_ULPI_REGADDR, bAddr);
        MGC_Write8(pBase, MGC_O_HDRC_ULPI_REGCTL,
                   MGC_M_ULPI_REGCTL_READNOTWRITE | MGC_M_ULPI_REGCTL_REG);
        while (!(MGC_M_ULPI_REGCTL_COMPLETE & bCtl) && (--retry)) {
                bCtl = MGC_Read8(pBase, MGC_O_HDRC_ULPI_REGCTL);
        }

        *pbData = MGC_Read8(pBase, MGC_O_HDRC_ULPI_REGDATA);
        MGC_Write8(pBase, MGC_O_HDRC_ULPI_REGCTL, 0);

        return TRUE;
}

uint8_t mgc_hdrc_write_ulpi(MGC_LinuxCd* pThis, uint8_t bAddr, uint8_t bData)
{
        uint8_t bCtl = 0, retry = 128;
        uint8_t* pBase = pThis->pRegs;

        /* ensure not powered down */
        if (MGC_Read8(pBase, MGC_O_HDRC_POWER) & MGC_M_POWER_ENSUSPEND) {
                return FALSE;
        }

        /* polled */
        MGC_Write8(pBase, MGC_O_HDRC_ULPI_REGADDR, bAddr);
        MGC_Write8(pBase, MGC_O_HDRC_ULPI_REGDATA, bData);
        MGC_Write8(pBase, MGC_O_HDRC_ULPI_REGCTL, MGC_M_ULPI_REGCTL_REG);

        while (!(MGC_M_ULPI_REGCTL_COMPLETE & bCtl) && (--retry)) {
                bCtl = MGC_Read8(pBase, MGC_O_HDRC_ULPI_REGCTL);
        }

        MGC_Write8(pBase, MGC_O_HDRC_ULPI_REGCTL, 0);

        return TRUE;
}

#define ULPI_FUN_CTL      	0x04
#define REG_LPI_VBUS      	0x70
#define REG_LPI_CARCTRL   	0x71
#define ULPI_IF_CTL       	0x07
#define ULPI_OTG_CTL      	0x0a
#define REG_LPI_ADDR     	0x75
#define REG_LPI_DATA      	0x74
#define REG_LPI_CTRL      	0x76

static void mgc_setting_ulpi(MGC_LinuxCd* pThis)
{
		//unsigned int addr = 0;
		//uint8_t tmp = 0;

#if 1 /* for orion_1.4 */
        MGC_Write8(pThis, 0x70, 0x01);//MGC_Write8(pThis, 0x70, 0x03);	u-boot: cmd_testusb.c
#endif

		mdelay(50);	//u-boot: cmd_testusb.c

#if 0	//u-boot: cmd_testusb.c
        addr = ULPI_FUN_CTL+1; /* write the function RESET regs, or with the write data */
        tmp = 0x20;
        mgc_hdrc_write_ulpi(pThis, addr, tmp);
        udelay(1000);	

        tmp = MGC_Read8(pThis->pRegs, REG_LPI_VBUS);

        /* set the DRV_VBUS_EXT = 1 */
        addr = ULPI_OTG_CTL;
        mgc_hdrc_read_ulpi(pThis, addr, &tmp);
        tmp |= 0x40;
        mgc_hdrc_write_ulpi(pThis, addr, tmp);

        /* set the USEExternalVbusIndicator = 1 */
        mgc_hdrc_read_ulpi(pThis, addr, &tmp);
        tmp |= 0x80;
        mgc_hdrc_write_ulpi(pThis, addr, tmp);

        /* set the DRV_VBUS = 1, Turn on the PHY led */
        mgc_hdrc_read_ulpi(pThis, addr, &tmp);
        tmp |= 0x20;
        mgc_hdrc_write_ulpi(pThis, addr, tmp);

        /* set the DPpulldown and Dmpulldown = 11 */
        mgc_hdrc_read_ulpi(pThis, addr, &tmp);
        tmp |= 0x06;
        mgc_hdrc_write_ulpi(pThis, addr, tmp);

        /* set the TermSelect = 1 */
        addr = ULPI_FUN_CTL;
        mgc_hdrc_read_ulpi(pThis, addr, &tmp);
        tmp |= 0x04;
        mgc_hdrc_write_ulpi(pThis, addr, tmp);

        /* set the OP Mode = 0 */
        mgc_hdrc_read_ulpi(pThis, addr, &tmp);
        tmp |= 0xe7;
        mgc_hdrc_write_ulpi(pThis, addr, tmp);

        /* set the XcvrSelect[0] = 1 */
        mgc_hdrc_read_ulpi(pThis, addr, &tmp);
        tmp |= 0x01;
        mgc_hdrc_write_ulpi(pThis, addr, tmp);

        /* set the Indicator Pass Thru  = 1  and set Indicator Complement */
        addr = ULPI_IF_CTL;
        mgc_hdrc_read_ulpi(pThis, addr, &tmp);
        tmp |= 0x40;
        tmp &= 0xdf;
        mgc_hdrc_write_ulpi(pThis, addr, tmp);
#endif				

        return;
}

/* ------------------------------------------------------------------------ */

/**
 * Discover HDRC configuration
 */
static uint8_t mgc_hdrc_init(uint16_t wType, MGC_LinuxCd* pThis)
{
#ifdef MGC_C_DYNFIFO_DEF
        uint16_t wFifoOffset;
#endif
        MGC_LinuxLocalEnd* pEnd;
        uint8_t bEnd;
        uint16_t wRelease, wRelMajor, wRelMinor;
        char aInfo[78];
        char aRevision[32];
        uint8_t	reg;	/* for reading FIFOSIZE register */
        uint8_t bType = 0;
        void* pBase = pThis->pRegs;
#ifndef MGC_C_DYNFIFO_DEF
        /* how many of a given size/direction found: */
        uint8_t b2kTxEndCount = 0;
        uint8_t b2kRxEndCount = 0;
        uint8_t b1kTxEndCount = 0;
        uint8_t b1kRxEndCount = 0;
        /* the smallest 2k or 1k ends in Tx or Rx direction: */
        uint8_t b2kTxEnd = 0;
        uint8_t b2kRxEnd = 0;
        uint8_t b1kTxEnd = 0;
        uint8_t b1kRxEnd = 0;
        /* for tracking smallest: */
        uint16_t w2kTxSize = 0;
        uint16_t w1kTxSize = 0;
        uint16_t w2kRxSize = 0;
        uint16_t w1kRxSize = 0;
#endif	/* !MGC_C_DYNFIFO_DEF */

        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

        /* log core options */
        MGC_SelectEnd(pBase, 0);
        reg = MGC_ReadCsr8(pBase, MGC_O_HDRC_CONFIGDATA, 0);

        strcpy(aInfo,(reg & MGC_M_CONFIGDATA_UTMIDW)?"UTMI-16":"UTMI-8");
        if (reg & MGC_M_CONFIGDATA_DYNFIFO) {
                strcat(aInfo, ", dyn FIFOs");
        }
        if (reg & MGC_M_CONFIGDATA_MPRXE) {
                strcat(aInfo, ", bulk combine");
        }
        if (reg & MGC_M_CONFIGDATA_MPTXE) {
                strcat(aInfo, ", bulk split");
        }
        if (reg & MGC_M_CONFIGDATA_HBRXE) {
                strcat(aInfo, ", HB-ISO Rx");
        }
        if (reg & MGC_M_CONFIGDATA_HBTXE) {
                strcat(aInfo, ", HB-ISO Tx");
        }
        if (reg & MGC_M_CONFIGDATA_SOFTCONE) {
                strcat(aInfo, ", SoftConn");
        }

        /* log release info */
        wRelease = MGC_Read16(pBase, 0x6c);
        pThis->bIsMultipoint = (MGC_CONTROLLER_MHDRC == wType) ? TRUE : FALSE;
		 if(pThis->bIsMultipoint )
		 	bType = 'M';
		 else
		 	bType = ' ';

        wRelMajor = (wRelease >> 10) & 0x1f;
        wRelMinor = wRelease & 0x3ff;
        snprintf(aRevision, 32, "%d.%d%s", wRelMajor,
                 wRelMinor, (wRelease & 0x8000) ? "RC" : "");
        printk(KERN_INFO "%s: %cHDRC version %s  info: %s\n", __FUNCTION__,
               bType, aRevision, aInfo);

#ifdef MGC_C_DYNFIFO_DEF
        if (!(reg & MGC_M_CONFIGDATA_DYNFIFO)) {
                printk(KERN_ERR "%s: Built for Dynamic FIFOs but not detected in hardware; please rebuild\n",
                       __FUNCTION__);
                return FALSE;
        }
#else
        if (reg & MGC_M_CONFIGDATA_DYNFIFO) {
                printk(KERN_ERR "%s: Dynamic FIFOs detected but support not built; please rebuild\n",
                       __FUNCTION__);
                return FALSE;
        }
#endif

        pThis->bBulkTxEnd = 0;
        pThis->bBulkRxEnd = 0;

#ifdef MGC_C_DYNFIFO_DEF
        /* Dynamic FIFO sizing: use pre-computed values */
        MGC_SelectEnd(pBase, 0);
        MGC_Write8(pBase, MGC_O_HDRC_TXFIFOSZ, 3);
        MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, 3);
        MGC_Write16(pBase, MGC_O_HDRC_TXFIFOADD, 0);
        MGC_Write16(pBase, MGC_O_HDRC_RXFIFOADD, 0);

        wFifoOffset = MGC_END0_FIFOSIZE;
        bEnd = 1;
        MGC_SelectEnd(pBase, bEnd);
        pEnd = &(pThis->aLocalEnd[bEnd]);
		 pEnd->bEnd = bEnd;

		  /* reserve bulk */
#if	0
        pThis->bBulkTxEnd = bEnd;
        pEnd->wMaxPacketSizeTx = 1 << ((MGC_BLK_SZ & 0xf)+3+(MGC_BLK_SZ>>4));
		 pEnd->wMaxPacketSizeRx = 0;
		 pEnd->bIsSharedFifo = FALSE;
        MGC_Write8(pBase, MGC_O_HDRC_TXFIFOSZ, MGC_BLK_SZ);
        MGC_Write16(pBase, MGC_O_HDRC_TXFIFOADD, wFifoOffset >> 3);
        wFifoOffset += pEnd->wMaxPacketSizeTx;
        bEnd++;	
        MGC_SelectEnd(pBase, bEnd);
        pEnd = &(pThis->aLocalEnd[bEnd]);
		 pEnd->bEnd = bEnd;
        pEnd->wMaxPacketSizeRx = 1 << ((MGC_BLK_SZ & 0xf)+3+(MGC_BLK_SZ>>4));
		 pEnd->wMaxPacketSizeTx = 0;
		 pEnd->bIsSharedFifo = FALSE;
        MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, MGC_BLK_SZ);
        MGC_Write16(pBase, MGC_O_HDRC_RXFIFOADD, wFifoOffset >> 3);
        pThis->bBulkRxEnd = bEnd;
        /* move to next */
        wFifoOffset += pEnd->wMaxPacketSizeRx;
        bEnd++;
		 MGC_SelectEnd(pBase, bEnd);
		 pEnd = &(pThis->aLocalEnd[bEnd]);
		 pEnd->bEnd = bEnd;
#endif
		 pThis->bBulkTxEnd = bEnd;
		 pThis->bBulkRxEnd = bEnd;
        pEnd->wMaxPacketSizeTx = 1 << ((MGC_BLK_SZ & 0xf)+3+(MGC_BLK_SZ>>4));
		 pEnd->wMaxPacketSizeRx = 1 << ((MGC_BLK_SZ & 0xf)+3+(MGC_BLK_SZ>>4));
		 pEnd->bIsSharedFifo = TRUE;
        MGC_Write8(pBase, MGC_O_HDRC_TXFIFOSZ, MGC_BLK_SZ);
        MGC_Write16(pBase, MGC_O_HDRC_TXFIFOADD, wFifoOffset >> 3);
        MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, MGC_BLK_SZ);
        MGC_Write16(pBase, MGC_O_HDRC_RXFIFOADD, wFifoOffset >> 3);
        /* move to next */
        wFifoOffset += pEnd->wMaxPacketSizeRx;
        bEnd++;
		 MGC_SelectEnd(pBase, bEnd);
		 pEnd = &(pThis->aLocalEnd[bEnd]);
		 pEnd->bEnd = bEnd;
		 
		 #if 0	/* MGC_DFIFO_INT_TX */
        /* reserve INT Tx */
		 pThis->bIntTxEnd = bEnd;
        MGC_Write8(pBase, MGC_O_HDRC_TXFIFOSZ, MGC_INT_TX_SZ);
        pEnd->wMaxPacketSizeTx = 1 << ((MGC_INT_TX_SZ & 0xf)+3+(MGC_INT_TX_SZ>>4));
        pEnd->wMaxPacketSizeRx = 0;
        pEnd->bIsSharedFifo = FALSE;
        MGC_Write16(pBase, MGC_O_HDRC_TXFIFOADD, wFifoOffset >> 3);
        /* move to next */
        wFifoOffset += pEnd->wMaxPacketSizeTx;
        bEnd++;
        MGC_SelectEnd(pBase, bEnd);
        pEnd = &(pThis->aLocalEnd[bEnd]);
		 pEnd->bEnd = bEnd;
#endif

#if 1	/* MGC_DFIFO_INT_RX */
        /* reserve INT Rx */
		 pThis->bIntRxEnd = bEnd;
        MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, MGC_INT_RX_SZ);
        pEnd->wMaxPacketSizeTx = 0;
        pEnd->wMaxPacketSizeRx = 1 << ((MGC_INT_RX_SZ & 0xf)+3+(MGC_INT_RX_SZ>>4));
        pEnd->bIsSharedFifo = FALSE;
        MGC_Write16(pBase, MGC_O_HDRC_RXFIFOADD, wFifoOffset >> 3);
        /* move to next */
        wFifoOffset += pEnd->wMaxPacketSizeRx;
        bEnd++;
        MGC_SelectEnd(pBase, bEnd);
        pEnd = &(pThis->aLocalEnd[bEnd]);
		 pEnd->bEnd = bEnd;
#endif
		 
#if 0	/* MGC_DFIFO_ISO_TX */
        /* reserve ISO Tx */
		 pThis->bIsoTxEnd = bEnd;
        MGC_Write8(pBase, MGC_O_HDRC_TXFIFOSZ, MGC_ISO_TX_SZ);
        pEnd->wMaxPacketSizeTx = 1 << ((MGC_ISO_TX_SZ & 0xf)+3+(MGC_ISO_TX_SZ>>4));
        pEnd->wMaxPacketSizeRx = 0;
        pEnd->bIsSharedFifo = FALSE;
        MGC_Write16(pBase, MGC_O_HDRC_TXFIFOADD, wFifoOffset >> 3);
        /* move to next */
        wFifoOffset += pEnd->wMaxPacketSizeTx;
        bEnd++;
        MGC_SelectEnd(pBase, bEnd);
        pEnd = &(pThis->aLocalEnd[bEnd]);
		 pEnd->bEnd = bEnd;
#endif
#if 0	/* MGC_DFIFO_ISO_RX */
        /* reserve ISO Rx */
		 pThis->bIsoRxEnd = bEnd;
        MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, MGC_ISO_RX_SZ);
        pEnd->wMaxPacketSizeTx = 0;
        pEnd->wMaxPacketSizeRx = 1 << ((MGC_ISO_RX_SZ & 0xf)+3+(MGC_ISO_RX_SZ>>4));
        pEnd->bIsSharedFifo = FALSE;
        MGC_Write16(pBase, MGC_O_HDRC_RXFIFOADD, wFifoOffset >> 3);
        /* move to next */
        wFifoOffset += pEnd->wMaxPacketSizeRx;
        bEnd++;
        //MGC_SelectEnd(pBase, bEnd);
        //pEnd = &(pThis->aLocalEnd[bEnd]);
		 //pEnd->bEnd = bEnd;
#endif
		 pThis->bIsoTxEnd = bEnd;
		 pThis->bIsoRxEnd = bEnd;
        MGC_Write8(pBase, MGC_O_HDRC_TXFIFOSZ, MGC_ISO_TX_SZ);
		 MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, MGC_ISO_RX_SZ);
        pEnd->wMaxPacketSizeTx = 1 << ((MGC_ISO_TX_SZ & 0xf)+3+(MGC_ISO_TX_SZ>>4));
        pEnd->wMaxPacketSizeRx = 1 << ((MGC_ISO_TX_SZ & 0xf)+3+(MGC_ISO_TX_SZ>>4));
        pEnd->bIsSharedFifo = TRUE;
        MGC_Write16(pBase, MGC_O_HDRC_TXFIFOADD, wFifoOffset >> 3);
		 MGC_Write16(pBase, MGC_O_HDRC_RXFIFOADD, wFifoOffset >> 3);
        /* move to next */
        wFifoOffset += pEnd->wMaxPacketSizeTx;
        bEnd++;
        //MGC_SelectEnd(pBase, bEnd);
        //pEnd = &(pThis->aLocalEnd[bEnd]);
		 //pEnd->bEnd = bEnd;
				
        /* finish */
        for (; bEnd < MGC_C_NUM_EPS; bEnd++) {
                MGC_SelectEnd(pBase, bEnd);
                pEnd = &(pThis->aLocalEnd[bEnd]);
				  pEnd->bEnd = bEnd;
                MGC_Write8(pBase, MGC_O_HDRC_TXFIFOSZ, MGC_DFIFO_ALL_VAL);
                MGC_Write8(pBase, MGC_O_HDRC_RXFIFOSZ, MGC_DFIFO_ALL_VAL);
                pEnd->wMaxPacketSizeTx = pEnd->wMaxPacketSizeRx = 1 << (MGC_DFIFO_ALL_VAL+3);
                pEnd->bIsSharedFifo = TRUE;
                MGC_Write16(pBase, MGC_O_HDRC_TXFIFOADD, wFifoOffset >> 3);
                MGC_Write16(pBase, MGC_O_HDRC_RXFIFOADD, wFifoOffset >> 3);
                wFifoOffset += pEnd->wMaxPacketSizeRx;
        }

#endif

        /* discover endpoint configuration */
        pThis->bEndCount = 1;
        pThis->wEndMask = 1;
        pEnd = &(pThis->aLocalEnd[0]);
		 pEnd->bEnd = 0;
        spin_lock_init(&pEnd->Lock);
        INIT_LIST_HEAD(&(pEnd->urb_list));
        pEnd->wMaxPacketSizeTx = MGC_END0_FIFOSIZE;
        pEnd->wMaxPacketSizeRx = MGC_END0_FIFOSIZE;
        pEnd->bLocalEnd = 0;
#if MGC_DEBUG > 0
        pEnd->dwPadFront = MGC_PAD_FRONT;
        pEnd->dwPadBack = MGC_PAD_BACK;
#endif

        for (bEnd = 1; bEnd < MGC_C_NUM_EPS; bEnd++) {
                MGC_SelectEnd(pBase, bEnd);
                pEnd = &(pThis->aLocalEnd[bEnd]);
                spin_lock_init( &pEnd->Lock );
                INIT_LIST_HEAD(&(pEnd->urb_list));
                pEnd->bLocalEnd = bEnd;

#if MGC_DEBUG > 0
                pEnd->dwPadFront = MGC_PAD_FRONT;
                pEnd->dwPadBack = MGC_PAD_BACK;
#endif

#ifndef MGC_C_DYNFIFO_DEF
                /* read from core */
                reg = MGC_ReadCsr8(pBase, MGC_O_HDRC_FIFOSIZE, bEnd);
                if (!reg) {
                        /* 0's returned when no more endpoints */
                        break;
                }
                pEnd->wMaxPacketSizeTx = 1 << (reg & 0x0f);
                /* shared TX/RX FIFO? */
                if ((reg & 0xf0) == 0xf0) {
                        pEnd->wMaxPacketSizeRx = 1 << (reg & 0x0f);
                        pEnd->bIsSharedFifo = TRUE;
                } else {
                        pEnd->wMaxPacketSizeRx = 1 << ((reg & 0xf0) >> 4);
                        pEnd->bIsSharedFifo = FALSE;
                }

                /* track certain sizes to try to reserve a bulk resource */
                if (pEnd->wMaxPacketSizeTx >= 2048) {
                        b2kTxEndCount++;
                        if (!b2kTxEnd || (pEnd->wMaxPacketSizeTx < w2kTxSize)) {
                                b2kTxEnd = bEnd;
                                w2kTxSize = pEnd->wMaxPacketSizeTx;
                        }
                }
                if (pEnd->wMaxPacketSizeRx >= 2048) {
                        b2kRxEndCount++;
                        if (!b2kRxEnd || (pEnd->wMaxPacketSizeRx < w2kRxSize)) {
                                b2kRxEnd = bEnd;
                                w2kRxSize = pEnd->wMaxPacketSizeRx;
                        }
                }
                if (pEnd->wMaxPacketSizeTx >= 1024) {
                        b1kTxEndCount++;
                        if (!b1kTxEnd || (pEnd->wMaxPacketSizeTx < w1kTxSize)) {
                                b1kTxEnd = bEnd;
                                w1kTxSize = pEnd->wMaxPacketSizeTx;
                        }
                }
                if (pEnd->wMaxPacketSizeRx >= 1024) {
                        b1kRxEndCount++;
                        if (!b1kRxEnd || (pEnd->wMaxPacketSizeRx < w1kTxSize)) {
                                b1kRxEnd = bEnd;
                                w1kRxSize = pEnd->wMaxPacketSizeRx;
                        }
                }
#endif	/* !MGC_C_DYNFIFO_DEF */

                pThis->bEndCount++;
                pThis->wEndMask |= (1 << bEnd);
        }

#ifndef MGC_C_DYNFIFO_DEF
        /* if possible, reserve the smallest 2k-capable Tx end for bulk */
        if (b2kTxEnd && (b2kTxEndCount > 1)) {
                pThis->bBulkTxEnd = b2kTxEnd;
                printk(KERN_INFO "%s: Reserved end %d for bulk double-buffered Tx\n",
                       __FUNCTION__, b2kTxEnd);
        }
        /* ...or try 1k */
        else if (b1kTxEnd && (b1kTxEndCount > 1)) {
                pThis->bBulkTxEnd = b1kTxEnd;
                printk(KERN_INFO "%s: Reserved end %d for bulk Tx\n",
                       __FUNCTION__, b1kTxEnd);
        }

        /* if possible, reserve the smallest 2k-capable Rx end for bulk */
        if (b2kRxEnd && (b2kRxEndCount > 1)) {
                pThis->bBulkRxEnd = b2kRxEnd;
                printk(KERN_INFO "%s: Reserved end %d for bulk double-buffered Rx\n",
                       __FUNCTION__, b2kRxEnd);
        }
        /* ...or try 1k */
        else if (b1kRxEnd && (b1kRxEndCount > 1)) {
                pThis->bBulkRxEnd = b1kRxEnd;
                printk(KERN_INFO "%s: Reserved end %d for bulk Rx\n",
                       __FUNCTION__, b1kRxEnd);
        }

#endif	/* !MGC_C_DYNFIFO_DEF */

        if (pThis->bBulkTxEnd) {
                pEnd = &(pThis->aLocalEnd[pThis->bBulkTxEnd]);
                pEnd->bIsClaimed = TRUE;
        }
        if (pThis->bBulkRxEnd) {
                pEnd = &(pThis->aLocalEnd[pThis->bBulkRxEnd]);
                pEnd->bIsClaimed = TRUE;
        }
		  if (pThis->bIsoTxEnd) {
                pEnd = &(pThis->aLocalEnd[pThis->bIsoTxEnd]);
                pEnd->bIsClaimed = TRUE;
        }
        if (pThis->bIsoRxEnd) {
                pEnd = &(pThis->aLocalEnd[pThis->bIsoRxEnd]);
                pEnd->bIsClaimed = TRUE;
        }
		  if (pThis->bIntTxEnd) {
                pEnd = &(pThis->aLocalEnd[pThis->bIntTxEnd]);
                pEnd->bIsClaimed = TRUE;
        }
        if (pThis->bIntRxEnd) {
                pEnd = &(pThis->aLocalEnd[pThis->bIntRxEnd]);
                pEnd->bIsClaimed = TRUE;
        }

        return TRUE;
}

#if MGC_DEBUG > 0
static uint8_t mgc_is_corrupt(MGC_LinuxCd* pThis)
{
        uint8_t bEnd;
        MGC_LinuxLocalEnd* pEnd;
        uint8_t bResult = FALSE;

        if (MGC_PAD_FRONT != pThis->dwPadFront) {
                printk(KERN_INFO "musb: front pad corrupted (%x)\n", pThis->dwPadFront);
                bResult = TRUE;
        }
        if (MGC_PAD_BACK != pThis->dwPadBack) {
                printk(KERN_INFO "musb: back pad corrupted (%x)\n", pThis->dwPadBack);
                bResult = TRUE;
        }
        for (bEnd = 0; bEnd < pThis->bEndCount; bEnd++) {
                pEnd = &(pThis->aLocalEnd[bEnd]);
                if (MGC_PAD_FRONT != pEnd->dwPadFront) {
                        printk(KERN_INFO "musb: end %d front pad corrupted (%x)\n",
                               bEnd, pEnd->dwPadFront);
                        bResult = TRUE;
                }
                if (MGC_PAD_BACK != pEnd->dwPadBack) {
                        printk(KERN_INFO "musb: end %d back pad corrupted (%x)\n",
                               bEnd, pEnd->dwPadBack);
                        bResult = TRUE;
                }
        }
        return bResult;
}
#endif

/**
 * Generic timer creation.
 * @param pThis instance pointer
 * @param pfFunc timer fire callback
 * @param pParam parameter for callback
 * @param millisecs how many milliseconds to set
 */
static void mgc_set_timer(MGC_LinuxCd* pThis,
                          void (*pfFunc)(unsigned long),
                          unsigned long pParam, unsigned long millisecs)
{
        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

        pThis->Timer.function = pfFunc;
        pThis->Timer.data = (unsigned long)pParam;
        mod_timer( &(pThis->Timer), jiffies + (HZ * millisecs) / 1000);
}

/* Allocate memory for a buffer that might use DMA.
 *
 * @param dma NULL when DMAble memeory is not requested.
 */
void* mgc_alloc_membuf(MGC_LinuxCd* pThis, size_t bytes,
                       int gfp_flags, dma_addr_t* dma)
{
        void* addr = NULL;

        if (dma) {
                *dma = DMA_ADDR_INVALID;
        }

        addr = kmalloc(bytes, gfp_flags);
        if ( addr && dma ) {
                *dma = virt_to_phys(addr);
        }

        DEBUG_CODE(2, printk(KERN_INFO "mallocd addr=%p, pDmaAddress=%p, len = %d\n", addr, dma, bytes); )

        return addr;
}

/* Free memory previously allocated with AllocBufferMemory.
 */
void mgc_free_membuf(MGC_LinuxCd* pThis, size_t bytes,
                     void *address, dma_addr_t dma)
{
        DEBUG_CODE(2, printk(KERN_INFO "<== freeing bytes=%d, address=%p, dma=%p\n", bytes, address, (void*)dma); )
        kfree(address);
}

/**
 * Allocate DMA buffer
 */
/*static void* mgc_buffer_alloc(struct usb_bus* pBus, size_t nSize,
                              int iMemFlags, dma_addr_t* pDmaAddress)
{
        MGC_LinuxCd* pThis=(MGC_LinuxCd*)pBus->hcpriv;

        return mgc_alloc_membuf(pThis, nSize, iMemFlags, pDmaAddress);
}*/

/**
 * Free DMA buffer
 */
/*static void mgc_buffer_free(struct usb_bus* pBus, size_t nSize,
                            void* address, dma_addr_t DmaAddress)
{
        MGC_LinuxCd* pThis=(MGC_LinuxCd*)pBus->hcpriv;

        mgc_free_membuf(pThis, nSize, address, DmaAddress);
}*/

// DELME /**
// DELME  * Disable an endpoint
// DELME  */
// DELME static void mgc_end_disable(struct usb_device* pDevice,
// DELME                             int bEndpointAddress)
// DELME {
// DELME         /* nothing to do */
// DELME }

// DELME /**
// DELME  * Private per-device allocation
// DELME  * @param pDevice Linux USBD device pointer
// DELME  * @return status code
// DELME  */
// DELME static int mgc_alloc_device(struct usb_device *pDevice)
// DELME {
// DELME 
// DELME         DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
// DELME 
// DELME         return 0;
// DELME 
// DELME }
// DELME 
// DELME /**
// DELME  * Private per-device cleanup
// DELME  * @param pDevice Linux USBD device pointer
// DELME  * @return 0 (success)
// DELME  */
// DELME static int mgc_free_device (struct usb_device * pDevice)
// DELME {
// DELME         DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )
// DELME 
// DELME         return 0;
// DELME }

/**************************************************************************
 * Linux driver hooks, including PCI
 **************************************************************************/

#ifdef MGC_DMA
static uint8_t mgc_dma_status_changed(void* pPrivateData, 
	uint8_t bLocalEnd, uint8_t bTransmit)
{
	return TRUE;
}

static irqreturn_t mgc_dma_isr(int irq, void *__hci, struct pt_regs *r)
{
        MGC_LinuxCd* pThis = (MGC_LinuxCd*)__hci;
	pThis->pDmaController->pfDmaControllerIsr(pThis->pDmaController->pPrivateData);
	
	RETURN_IRQ_HANDLED;
}
#endif

/**
 * ISR
 * @param irq interrupt line associated with the controller
 * @param hci data structure for the host controller
 * @param r holds the snapshot of the processor's context before
 *             the processor entered interrupt code. (not used here)
 */
static irqreturn_t mgc_controller_isr(int irq, void *__hci, struct pt_regs *r)
{
        uint8_t bShift;
        uint32_t nSource;
        uint32_t reg;
        uint8_t bIntrUsbValue;
        uint16_t wIntrTxValue, wIntrRxValue;
        MGC_LinuxCd* pThis = (MGC_LinuxCd*)__hci;
        const void* pBase = pThis->pRegs;
	
        uint8_t devctl;

#if 1
		if (UsbDviceNotConnected)
		{
			// mdelay(20000);
		}
#endif
		devctl = MGC_Read8(pBase, MGC_O_HDRC_DEVCTL);

		bIntrUsbValue = MGC_Read8(pBase, MGC_O_HDRC_INTRUSB);
        /* read registers (this clear the interrupt) */
		// printk("mgc_controller_isr : bIntrUsbValue[0x%02X] \n", bIntrUsbValue);
		
        wIntrTxValue = MGC_Read16(pBase, MGC_O_HDRC_INTRTX);
        wIntrRxValue = MGC_Read16(pBase, MGC_O_HDRC_INTRRX);

        /* hook extra pre-ISR handling */
        MGC_EXTRA_PRE_ISR(pThis);

        /* determine if we were the cause; return if not */
        nSource = bIntrUsbValue | wIntrTxValue | wIntrRxValue;
        DEBUG_CODE(10,  if ( !nSource) {
        \
        printk(KERN_INFO "%s: IRQ [mode=%s] nSource=%d\n", \
               __FUNCTION__, MGC_MODE(pThis), nSource);
        })

        DEBUG_CODE(3, \
                   printk(KERN_INFO "%s[%ld]: IRQ RECEIVED [mode=%s] IntrUSB=%02x, IntrTx=%04x, IntrRx=%04x\n", \
                          __FUNCTION__, jiffies, MGC_MODE(pThis), bIntrUsbValue, wIntrTxValue, wIntrRxValue); )

#ifdef MGC_PARANOID
        /* corruption check */
        if (mgc_is_corrupt(pThis)) {
                printk(KERN_INFO "stopping before ISR, the controller structure is corrupted\n");
                mgc_hdrc_stop(pThis);
                MGC_ERR_MODE(pThis);

                RETURN_IRQ_HANDLED;
        }
#endif

        /* ignore requests when in error */
        if (MGC_IS_ERR(pThis)) {
                RETURN_IRQ_HANDLED;
        }

        /*
         * the core can interrupt us for multiple reasons, I.E. more than an
         * interrupt line can be asserted; service the globa interrupt first.
         * Global interrups are used to signal
         */
        if (bIntrUsbValue) {
			// printk ("mgc_controller_isr : 0x%02X\n", bIntrUsbValue);
			mgc_hdrc_service_usb(pThis, bIntrUsbValue);
        }

        /* each bit of wIntrTxValue represent an endpoint, EP0 gets a "SPECIAL" treatment */
        reg = wIntrTxValue;
        /* EP0 */
        if (reg & 1) {
                if (devctl & MGC_M_DEVCTL_HM) {
                        mgc_hdrc_service_default_end(pThis);
                }
        }

        /* TX on endpoints 1-15 */
        bShift = 1;
        reg >>= 1;
        while (reg) {
                if (reg & 1) {
                        if (devctl & MGC_M_DEVCTL_HM) {
                                mgc_hdrc_service_tx_avail(pThis, bShift);
                        }
                }

                reg >>= 1;
                bShift++;
        }

        /* RX on endpoints 1-15 */
        reg = wIntrRxValue;
        bShift = 1;
        reg >>= 1;
        while (reg) {
                if (reg & 1) {
                        if (devctl & MGC_M_DEVCTL_HM) {

                                mgc_hdrc_service_rx_ready(pThis, bShift);
                        }
                }

                reg >>= 1;
                bShift++;
        }

#ifdef MGC_PARANOID
        if (mgc_is_corrupt(pThis)) {
                printk(KERN_INFO "stopping after ISR\n");
                mgc_hdrc_stop(pThis);
                MGC_ERR_MODE(pThis);
        }
#endif

        /* hook extra post-ISR handling */
        MGC_EXTRA_POST_ISR(pThis);

        DEBUG_CODE(2, printk(KERN_INFO "%s: IRQ HANDLED [mode=%s]\n", __FUNCTION__, MGC_MODE(pThis));)

        RETURN_IRQ_HANDLED;
}

/* the below function is defined in ../core/hcd.c */
extern void usb_deregister_bus (struct usb_bus *bus);
extern int usb_register_bus(struct usb_bus *bus);

/**
 * Perform generic per-controller initialization.
 * @param pDevice pci_dev on PCI systems
 * @param wType controller type
 * @param nIrq IRQ (interpretation is system-dependent)
 * @param pRegs pointer to controller registers,
 * assumed already mapped into kernel space
 * @param pName name for bus
 */
static MGC_LinuxCd* mgc_controller_init(void* pDevice, uint16_t wType,
                                        int nIrq, void* pRegs,
                                        const char* pName)
{
        uint8_t bOk;
        uint8_t bEnd;
        MGC_LinuxCd* pThis;
        MGC_LinuxLocalEnd* pEnd;
        //char buf[8];
        unsigned char *regs_base = NULL;

        request_mem_region((int)pRegs, 0x10000, "ORION USB");
        regs_base =ioremap((int)pRegs, 0x10000);

        //snprintf(buf, 8, "%d", nIrq);
    
        /* allocate */
        pThis = kmalloc(sizeof(MGC_LinuxCd), GFP_KERNEL);
        if (!pThis) {
                printk(KERN_ERR "%s: kmalloc driver instance data failed\n", __FUNCTION__);
                return NULL;
        }
        memset (pThis, 0, sizeof(MGC_LinuxCd));
#if MGC_DEBUG > 0
        pThis->dwPadFront = MGC_PAD_FRONT;
        pThis->dwPadBack = MGC_PAD_BACK;
#endif
        strcpy(pThis->aName, pName);
        pThis->pRegs = regs_base;
        spin_lock_init(&pThis->Lock);
        init_timer(&(pThis->Timer));

        printk(KERN_INFO "%s: Driver instance data at 0x%lx\n",
               __FUNCTION__, (unsigned long)pThis);

        /* be sure interrupts are disabled before connecting ISR */
        mgc_hdrc_disable(pThis);

        /* hook for extra pre-init handling */
        MGC_EXTRA_PRE_INIT(nIrq, pThis->pRegs);

        /* discover configuration */
        bOk = mgc_hdrc_init(wType, pThis);
        if ( !bOk ) {
                kfree(pThis);
                return NULL;
        }

        /* print config */
        for (bEnd = 0; bEnd < min((int)MGC_C_NUM_EPS, (int)pThis->bEndCount); bEnd++) {
                pEnd = &(pThis->aLocalEnd[bEnd]);
                if (pEnd->wMaxPacketSizeTx || pEnd->wMaxPacketSizeRx) {
                        printk(KERN_INFO "%s: End %02d: %sFIFO TxSize=%04x/RxSize=%04x\n",
                               __FUNCTION__, bEnd, pEnd->bIsSharedFifo ? "Shared " : "",
                               pEnd->wMaxPacketSizeTx, pEnd->wMaxPacketSizeRx);
                }
        }

        /* attach to the IRQ */
        if (request_irq (nIrq, mgc_controller_isr, /*SA_SHIRQ|*/SA_INTERRUPT, pThis->aName, pThis)) {
                err("request_irq %d failed!", nIrq);
                kfree(pThis);
                return NULL;
        }
        pThis->nIrq = nIrq;

#ifdef MGC_DMA
	if (request_irq (27, mgc_dma_isr, /*SA_SHIRQ|*/SA_INTERRUPT, "usb DMA controller", pThis)) {
		err("request_irq %d failed!", 27);
		free_irq(nIrq, pThis);
		kfree(pThis);

		return NULL;
	}

	pThis->pDmaController = MGC_HdrcDmaControllerFactory.pfNewDmaController(mgc_dma_status_changed, pThis, (uint8_t*)pThis->pRegs);
	if(pThis->pDmaController) {
		DEBUG_CODE(2, printk("DMA initialized&enabled Address: 0x%p\n", pThis->pDmaController);)
		pThis->pDmaController->pfDmaStartController(pThis->pDmaController->pPrivateData);
	}
#endif

        /* allocate and register bus */
        pThis->pBus = usb_alloc_bus(&mgc_ops);
        if (!pThis->pBus) {
                dbg ("usb_alloc_bus() fail");
                free_irq (nIrq, pThis);
                kfree(pThis);
                return NULL;
        }

#ifdef MGC_HAS_BUSNAME
        pThis->pBus->bus_name = pThis->aName;
#endif
        pThis->pBus->hcpriv = (void *)pThis;

        printk(KERN_INFO "%s: New bus @%#lx\n",
               __FUNCTION__, (unsigned long)pThis->pBus);

        /* register the bus */
        pThis->pBus->controller = (struct device*)pDevice;
        usb_register_bus(pThis->pBus);
		
        /* init virtual root hub */
        pThis->PortServices.pPrivateData = pThis;
        pThis->PortServices.pfSetPortPower = mgc_set_port_power;
        pThis->PortServices.pfSetPortEnable = mgc_set_port_enable;
        pThis->PortServices.pfSetPortSuspend = mgc_set_port_suspend;
        pThis->PortServices.pfSetPortReset = mgc_set_port_reset;

        bOk = MGC_VirtualHubInit(&(pThis->RootHub), pThis->pBus, 1, &(pThis->PortServices));
        if (!bOk) {
                dbg ("Virtual Hub init failed");
                free_irq (nIrq, pThis);
                kfree(pThis);
                return NULL;
        }

        pThis->pBus->root_hub = pThis->RootHub.pDevice;
        MGC_HST_MODE(pThis);

	mdelay(2);
        MGC_Write8(pThis->pRegs, MGC_O_HDRC_INTRUSBE, 0xf7); /*  enable all of interrupts, except for SOF interrupt */
	musb_buffer_create ( pThis);
        /* hook for extra post-init handling */
        MGC_EXTRA_POST_INIT(pThis);

        return pThis;
}

/*
 * Release resources acquired by driver
 */
static void mgc_controller_free(MGC_LinuxCd* pThis)
{
        DEBUG_CODE(2, printk(KERN_INFO "=> %s\n", __FUNCTION__); )

        mgc_hdrc_stop(pThis);
        MGC_ERR_MODE(pThis);

#ifdef MGC_DMA
	if(pThis->pDmaController) {
		pThis->pDmaController->pfDmaStopController(pThis->pDmaController->pPrivateData);
		MGC_HdrcDmaControllerFactory.pfDestroyDmaController(pThis->pDmaController);
	}
#endif
	
        MGC_VirtualHubStop(&pThis->RootHub);
        MGC_VirtualHubDestroy(&pThis->RootHub);

        if (pThis->pBus->root_hub) {
                usb_disconnect(&(pThis->pBus->root_hub));
        }

	mdelay(1);
        usb_deregister_bus(pThis->pBus);

        if (pThis->nIrq) {
                free_irq(pThis->nIrq, pThis);
        }
	
#ifdef MGC_DMA
	free_irq(27, pThis);
#endif

        kfree(pThis);
}

/******************************************************************
 * Driver init
 ******************************************************************/

#define kobj_to_direct_driver(obj) container_of(obj, struct device_driver, kobj)
#define attr_to_driver_attribute(obj) container_of(obj, struct driver_attribute, attr)

/* probably we don't need to be a system device in this case */
struct device mgc_device = { };
	int i_i;
struct device_driver mgc_driver= {
        .name = "musb-hcd",
        };

static inline ssize_t store_new_id(struct device_driver *driver, const char *buf, size_t count);

static ssize_t driver_count = 0;
static DRIVER_ATTR(new_id, S_IWUSR, NULL, store_new_id);

/*
 * store_new_id
 *
 * Adds a new dynamic pci device ID to this driver,
 * and causes the driver to probe for all devices again.
 */
static inline ssize_t store_new_id(struct device_driver *driver, const char *buf, size_t count)
{
        return driver_count++;
}

static ssize_t direct_driver_attr_store(struct kobject * kobj, struct attribute *attr,
                                        const char *buf, size_t count)
{
        struct device_driver *driver = (struct device_driver *)kobj_to_direct_driver(kobj);
        struct driver_attribute *dattr = (struct driver_attribute *)attr_to_driver_attribute(attr);
        ssize_t ret = 0;

        if (get_driver(driver)) {
                if (dattr->store)
                        ret = dattr->store(driver, buf, count);
                put_driver(driver);
        }

        return ret;
}

static ssize_t direct_driver_attr_show(struct kobject * kobj, struct attribute *attr, char *buf)
{
        ssize_t ret = 0;

        struct device_driver *driver = (struct device_driver *)kobj_to_direct_driver(kobj);
        struct driver_attribute *dattr = (struct driver_attribute *)attr_to_driver_attribute(attr);

        if ( get_driver(driver) ) {
                if (dattr->show)
                        ret = dattr->show(driver, buf);
                put_driver(driver);
        }

        return ret;
}

static struct sysfs_ops direct_driver_sysfs_ops = {
        .show = direct_driver_attr_show,
                .store = direct_driver_attr_store,
                 };

static struct kobj_type direct_driver_kobj_type = {
        .sysfs_ops = &direct_driver_sysfs_ops,
             };

static int direct_create_newid_file(struct device_driver *drv)
{
        int error = 0;
        if (drv->probe != NULL)
                error = sysfs_create_file(&drv->kobj,
                                          &driver_attr_new_id.attr);
        return error;
}

static int direct_populate_driver_dir(struct device_driver *drv)
{
        return direct_create_newid_file(drv);
}

static int direct_hotplug (struct device *dev, char **envp, int num_envp, char *buffer, int buffer_size)
{
        return -ENODEV;
}

static int direct_device_suspend(struct device * dev, u32 state)
{
        return 0;
}

static int direct_device_resume(struct device * dev)
{
        return 0;
}

static int direct_bus_match(struct device * dev, struct device_driver * drv)
{
        return (&mgc_driver==drv)?1:0;
}

struct bus_type direct_bus_type = {
        .name		= "system",
	.match		= direct_bus_match,
	.hotplug	= direct_hotplug,
	.suspend	= direct_device_suspend,
        .resume		= direct_device_resume,
};

/**
 * pci_register_driver - register a new pci driver
 * @drv: the driver structure to register
 *
 * Adds the driver structure to the list of registered drivers
 * Returns the number of pci devices which were claimed by the driver
 * during registration.  The driver remains registered even if the
 * return value is zero.
 */
static int direct_register_driver(struct device_driver *drv, struct bus_type *btype)
{
        int count = 0;

        /* initialize common driver fields */
        drv->bus = (btype)?btype:&direct_bus_type;
        drv->kobj.ktype = &direct_driver_kobj_type;

        /* register with core */
        count = driver_register( drv );
        if (count >= 0) {
                direct_populate_driver_dir( drv );
        }

        return count ? count : 1;
}

/**
 * unregister_driver - unregister a driver
 * @drv: the driver structure to unregister
 *
 * Deletes the driver structure from the list of registered drivers,
 * gives it a chance to clean up by calling its remove() function for
 * each device it was responsible for, and marks those devices as
 * driverless.
 */

static void direct_unregister_driver(struct device_driver *drv)
{
        driver_unregister( drv );
}

/* ------------------------------------------------------------------- */
/* ------------------------------------------------------------------- */


/*
 * Discover and initialize the drivers on the direct bus.
 */
static int direct_bus_init(void)
{
        int nIndex, rc = 0;
        char name[32];
        void* pDevice = NULL;

        const int nCount = sizeof(MGC_aLinuxController) / sizeof(MGC_LinuxController);

        if (!nCount) return 0;

        mgc_driver_instances = kmalloc(nCount*sizeof(MGC_LinuxCd*), GFP_ATOMIC);
        if (NULL == mgc_driver_instances) {
                return -ENOMEM;
        }

       pDevice = &mgc_device;
     
        kobject_set_name(&((struct device*)pDevice)->kobj, "musbdev");
        kobject_register(&((struct device*)pDevice)->kobj);

        INIT_LIST_HEAD(&((struct device*)pDevice)->children );

        ((struct device *)pDevice)->driver = &mgc_driver;

        sprintf(&((struct device *)pDevice)->bus_id[0], "usb%d", rc);
        bus_register(&direct_bus_type);
        direct_register_driver(&mgc_driver, NULL);

	
	 ((struct device*)pDevice)->dma_pools.next=(struct list_head*)(&i_i);
	  ((struct device*)pDevice)->coherent_dma_mask=~0; 
	  ((struct device*)pDevice)->dma_mask=&dma_mask; 
        for (nIndex = 0; !rc && nIndex < nCount; nIndex++) {
                MGC_LinuxController* pStaticController = &(MGC_aLinuxController[nIndex]);
                snprintf(name, 32, "musbhdrc%d", mgc_index++);
                mgc_driver_instances[nIndex] = mgc_controller_init(pDevice,
                                               pStaticController->wType, pStaticController->dwIrq,
                                               pStaticController->pBase, name);

                if (mgc_driver_instances[nIndex]) {
                        MGC_VirtualHubStart(&(mgc_driver_instances[nIndex]->RootHub));
                        mgc_instances_count++;
                } else {
                        printk(KERN_ERR "controller %d failed to initialize\n", nIndex);
                        direct_bus_shutdown();
			
			rc = -1;
                }
        }

        return mgc_instances_count;
}

/*
 *
 */
static void direct_bus_shutdown(void)
{
        int nIndex=0;

        for (nIndex = 0; nIndex < mgc_instances_count; nIndex++) {
                mgc_controller_free( mgc_driver_instances[nIndex] );
        }

        kfree(mgc_driver_instances);

        direct_unregister_driver(&mgc_driver);
}

/*-------------------------------------------------------------------------*/

/* this module is always GPL, the gadget might not... */
MODULE_DESCRIPTION (DRIVER_DESC);
MODULE_AUTHOR (DRIVER_AUTHOR);
MODULE_LICENSE ("GPL");

static struct proc_dir_entry *usb_proc_entry = NULL;

static int usb_proc_read(char *page, char **start, off_t off, int count,
			 int *eof, void *data)
{
	int len = count > 2 ? 2 : count;

	memcpy(page, usb_stat, len);

	return len;
}

/**
 * Required initialization for any module.
 */
int __init mgc_module_init(void)
{
        int rc = -ENODEV;
        const int nCount = sizeof(MGC_aLinuxController) / sizeof(MGC_LinuxController);

        printk(KERN_INFO "%s: Initializing MUSB Driver [npci=%d][gadget=%s][otg=%s]\n",
               __FILE__, nCount, "no","no");
	 printk("MUSB Drive version 1.1.0.1\n");//
        if (!usb_disabled()) {
                /* hook extra init */
                MGC_EXTRA_INIT();
                direct_bus_init();

                rc = 0;
        } else {
                printk(KERN_INFO "%s: USB Disabled; exiting\n", __FILE__);
        }

	usb_proc_entry = create_proc_entry("usb_conn_status", 0, NULL);
	if (NULL != usb_proc_entry) {
		usb_proc_entry->read_proc = &usb_proc_read;
	}

        return rc;
}
module_init (mgc_module_init);

/**
 * Required cleanup for any module
 */
void __exit mgc_module_cleanup(void)
{
        direct_bus_shutdown();
}
module_exit (mgc_module_cleanup);
