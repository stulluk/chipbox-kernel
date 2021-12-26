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
 * Inventra Controller Driver (ICD) for Linux.
 * A procfs-based High-Speed Electrical Test (HSET) tool
 * $Revision: 1.1.1.1 $
 */

#include <asm/uaccess.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/usb.h>

#ifdef MUSB_V26
#include <linux/device.h>
#endif

#include "musbdefs.h"

typedef enum
{
    MGC_TEST_NONE,
    MGC_TEST_J,
    MGC_TEST_K,
    MGC_TEST_SE0_NAK,
    MGC_TEST_PACKET,
    MGC_TEST_GET_DESC,
    MGC_TEST_SET_ADDR,
    MGC_TEST_STEP_SET_FEATURE,
    MGC_TEST_STEP_GET_DESC,
    MGC_TEST_ENABLE_WAKE,
    MGC_TEST_DISABLE_WAKE,
} MGC_Test;

typedef enum
{
    MGC_ERROR_STALL,
    MGC_ERROR_ERR,
    MGC_ERROR_NAK,
} MGC_Error;

typedef struct
{
    spinlock_t Lock;
    struct proc_dir_entry* pProcEntry;
    struct {
    	off_t nSize;
	char aBuffer[4*1024];
    } ProcData;
    MGC_LinuxCd* pCd;
    const uint8_t* pSetup;
    uint8_t aDescriptor[256];
    MGC_Test bTest;
    MGC_Error bErrorCode;
    uint8_t bRxSize;
    uint8_t bRxRemain;
    uint8_t bRxOffset;
    uint8_t bLoop;
    uint8_t bSingleStep;
    uint8_t bNoStatusPhase;
    uint8_t bPeerAddress;
    uint8_t bPeerSpeed;
    uint8_t bPeerHubAddr;
    uint8_t bPeerHubPort;
    uint8_t bTestAddress;
    uint8_t bIsSuspended;
    uint8_t bIsDisconnected;
    uint8_t bEnd0Stage;
    uint8_t bIsIn;
    uint8_t bError;
} MGC_TestProcData;

/******************************* FORWARDS ********************************/

static void MGC_TestProcServiceDefaultEnd(void* data);
#ifdef MUSB_V26
STATIC int MGC_TestModeDeviceProbe(struct usb_interface *intf, 
    const struct usb_device_id *id);
STATIC void MGC_TestModeDeviceDisconnect(struct usb_interface *intf);
#else
STATIC void* MGC_TestModeDeviceProbe(struct usb_device *dev, unsigned int i,
    const struct usb_device_id *id);
STATIC void MGC_TestModeDeviceDisconnect(struct usb_device *dev, void *ptr);
#endif

/******************************* GLOBALS *********************************/

/** SET_ADDRESS request template */
static uint8_t MGC_aSetDeviceAddress[] =
{
    0,
    5,
    0, 0, /* address */
    0, 0,
    0, 0
};

/** GET_DESCRIPTOR(DEVICE) request template */
static uint8_t MGC_aGetDeviceDescriptor[] =
{
    0x80,
    6, 
    0, 1, 
    0, 0,
    18, 0 /* allowed descriptor length */
};

/** SET_FEATURE(DEVICE) request template */
static uint8_t MGC_aSetTestMode[] =
{
    0,
    3, 
    2, 0, /* feature selector */
    0, 0,   /* test mode (upper byte) */
    0, 0
};

/** SET_FEATURE(DEVICE) request template */
static uint8_t MGC_aSetRemoteWake[] =
{
    0,
    3, 
    1, 0, /* feature selector */
    0, 0,
    0, 0
};

/** CLEAR_FEATURE(DEVICE) request template */
static uint8_t MGC_aClearRemoteWake[] =
{
    0,
    1, 
    1, 0, /* feature selector */
    0, 0,
    0, 0
};

/** mysterious SET_FEATURE() request template */
static uint8_t MGC_aSetFeature[] =
{
    0,
    3, 
    5, 0, /* feature selector */
    0, 0,
    0, 0
};

static const char* MGC_aTestName[] =
{
    "None",
    "TEST_J",
    "TEST_K",
    "TEST_SE0_NAK",
    "TEST_PACKET",
    "Get Device Descriptor",
    "Set Address",
    "Single-step Set-Feature",
    "Single-step Get-Device-Descriptor",
    "Enable Remote Wakeup",
    "Disable Remote Wakeup",
};

static const char* MGC_aPhaseName[] =
{
    "Setup",
    "Out",
    "In",
    "Status"
};

#ifdef MUSB_OTG
static const char* MGC_aOtgStateNameName[] =
{
    "AB-IDLE",
    "B-SRP-INIT",
    "B-PERIPHERAL",
    "B-WAIT-ACON",
    "B-HOST",
    "A-WAIT-BCON",
    "A-HOST",
    "A-SUSPEND",
    "A-PERIPH"
};

static const char* MGC_aOtgError[] =
{
    "",
    " (SRP failed)",
    " (Device not responding)",
    " (Device not supported)",
    " (Hubs not supported)"
};
#endif

static struct usb_device_id MGC_TestModeDeviceIdTable[] =
{
    {
	USB_DEVICE(6666, 0x0101)
    },
    {
	USB_DEVICE(6666, 0x0102)
    },
    {
	USB_DEVICE(6666, 0x0103)
    },
    {
	USB_DEVICE(6666, 0x0104)
    },
    {
	USB_DEVICE(6666, 0x0105)
    },
    {
	USB_DEVICE(6666, 0x0106)
    },
    {
	USB_DEVICE(6666, 0x0107)
    },
    {
	USB_DEVICE(6666, 0x0108)
    },
    { }						/* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, MGC_TestModeDeviceIdTable);

static struct usb_driver MGC_TestModeDeviceDriver = 
{
	name:		"TestModeDevice",
	probe:		MGC_TestModeDeviceProbe,
/*	ioctl:		MGC_TestModeDeviceIoctl, */
	disconnect:	MGC_TestModeDeviceDisconnect,
	id_table:	MGC_TestModeDeviceIdTable,
};

static MGC_TestProcData* MGC_pTestProcData;

/****************************** FUNCTIONS ********************************/

static void MGC_TestProcDisconnectListener(void* data)
{
    MGC_TestProcData* pThis=(MGC_TestProcData*)data;
    pThis->bIsDisconnected = TRUE;
}

/**
 * Start the setup in pThis->pSetup
 */
static void MGC_TestProcStartSetup(MGC_TestProcData* pThis)
{
    uint8_t bSpeed = pThis->bPeerSpeed;
    uint8_t bHubAddr = pThis->bPeerHubAddr;
    uint8_t bHubPort = pThis->bPeerHubPort;
    uint8_t reg = 0;
    uint8_t bIsMulti = FALSE;
    uint8_t* pBase=pThis->pCd->pRegs;

    if(pThis->pCd->bIsMultipoint)
    {
	switch(bSpeed)
	{
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

    pThis->bError = FALSE;
    pThis->bRxRemain = pThis->bRxSize;
    pThis->bRxOffset = 0;
    pThis->bEnd0Stage = MGC_END0_START;
    pThis->bIsIn = (pThis->pSetup[0] & 0x80) ? TRUE : FALSE;
    
    MGC_HdrcSetDefaultEndHandler(pThis->pCd, MGC_TestProcServiceDefaultEnd, pThis);

    MGC_SelectEnd(pBase, 0);

    if(pThis->pCd->bIsMultipoint)
    {
	printk(KERN_INFO "%s: multi-point core; SETUP to %02x (hub %02x, port %02x)\n", 
	    __FUNCTION__, pThis->bPeerAddress, bHubAddr, bHubPort);

	/* target addr & hub addr/port */
	MGC_Write8(pBase, MGC_BUSCTL_OFFSET(0, MGC_O_HDRC_TXFUNCADDR), 
	    pThis->bPeerAddress);
	MGC_Write8(pBase, MGC_BUSCTL_OFFSET(0, MGC_O_HDRC_TXHUBADDR), 
	    bIsMulti ? 0x80 | bHubAddr : bHubAddr);
	MGC_Write8(pBase, MGC_BUSCTL_OFFSET(0, MGC_O_HDRC_TXHUBPORT), 
	    bHubPort);
	/* also, try Rx */
	MGC_Write8(pBase, MGC_BUSCTL_OFFSET(0, MGC_O_HDRC_RXFUNCADDR), 
	    pThis->bPeerAddress);
	MGC_Write8(pBase, MGC_BUSCTL_OFFSET(0, MGC_O_HDRC_RXHUBADDR), 
	    bIsMulti ? 0x80 | bHubAddr : bHubAddr);
	MGC_Write8(pBase, MGC_BUSCTL_OFFSET(0, MGC_O_HDRC_RXHUBPORT), 
	    bHubPort);

	/* TxType */
	MGC_WriteCsr8(pBase, MGC_O_HDRC_TYPE0, 0, reg & 0xc0);
    }
    else
    {
	/* non-multipoint core */
	printk(KERN_INFO "%s: single-point core; SETUP to %02x\n", 
	    __FUNCTION__, pThis->bPeerAddress);
	MGC_Write8(pBase, MGC_O_HDRC_FADDR, pThis->bPeerAddress);
    }

    printk(KERN_INFO "%s: sending %02x %02x %02x %02x %02x %02x %02x %02x\n",
	__FUNCTION__,
	pThis->pSetup[0], pThis->pSetup[1], pThis->pSetup[2], pThis->pSetup[3], 
	pThis->pSetup[4], pThis->pSetup[5], pThis->pSetup[6], pThis->pSetup[7]);
#ifdef MUSB_DMA2
    MGC_HdrcLoadFifo(pThis, pBase, 0, 8, pThis->pSetup);
#else
    MGC_HdrcLoadFifo(pBase, 0, 8, pThis->pSetup);
#endif
    /* be careful; writes to upper bits will hang the core! */
    MGC_WriteCsr8(pBase, MGC_O_HDRC_NAKLIMIT0, 0, 16);

    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 
	MGC_M_CSR0_H_SETUPPKT | MGC_M_CSR0_TXPKTRDY);
}

/** Called from ISR or "Next" command to start next step */
static void MGC_TestProcNextStep(MGC_TestProcData* pThis)
{
    uint8_t* pBase=pThis->pCd->pRegs;

    /* based on last step */
    switch(pThis->bEnd0Stage)
    {
    case MGC_END0_START:
	/* we just started, so we just sent SETUP */
	if(pThis->bIsIn)
	{
	    /* in, so ask for packet */
	    printk(KERN_INFO "%s: requesting packet\n", __FUNCTION__);
	    pThis->bEnd0Stage = MGC_END0_IN;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_H_REQPKT);
	}
	else
	{
	    /* zero data request, so ask for status */
	    printk(KERN_INFO "%s: requesting status\n", __FUNCTION__);
	    pThis->bEnd0Stage = MGC_END0_STATUS;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 
		MGC_M_CSR0_H_STATUSPKT | MGC_M_CSR0_H_REQPKT);
	}
	break;

    case MGC_END0_IN:
	if(pThis->bRxRemain)
	{
	    /* in, so ask for packet */
	    printk(KERN_INFO "%s: requesting packet\n", __FUNCTION__);
	    pThis->bEnd0Stage = MGC_END0_IN;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_H_REQPKT);
	}
	else if(!pThis->bNoStatusPhase)
	{
	    /* we just got data, so send status */
	    printk(KERN_INFO "%s: sending status\n", __FUNCTION__);
	    pThis->bEnd0Stage = MGC_END0_STATUS;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 
		MGC_M_CSR0_H_STATUSPKT | MGC_M_CSR0_TXPKTRDY);
	}
	break;

    case MGC_END0_STATUS:
	/* we just did status, so if loop mode, start anew */
	printk(KERN_INFO "%s: status complete\n", __FUNCTION__);
	pThis->bSingleStep = FALSE;
	if(pThis->bLoop)
	{
	    MGC_TestProcStartSetup(pThis);
	}
	break;
    }
}

/** Service default endpoint */
static void MGC_TestProcServiceDefaultEnd(void* data)
{
    uint16_t wCsrVal, wCount;
    MGC_TestProcData* pThis=(MGC_TestProcData*)data;
    uint8_t* pBase=pThis->pCd->pRegs;

    MGC_SelectEnd(pBase, 0);
    wCsrVal = MGC_ReadCsr16(pBase, MGC_O_HDRC_CSR0, 0);
    wCount = MGC_ReadCsr8(pBase, MGC_O_HDRC_COUNT0, 0);

    /* check & handle errors */
    if(wCsrVal & MGC_M_CSR0_H_RXSTALL)
    {
	printk(KERN_INFO "%s: got STALL\n", __FUNCTION__);
	pThis->bError = TRUE;
	pThis->bErrorCode = MGC_ERROR_STALL;
    }
    else if(wCsrVal & MGC_M_CSR0_H_ERROR)
    {
	printk(KERN_INFO "%s: Rx Error\n", __FUNCTION__);
	pThis->bError = TRUE;
	pThis->bErrorCode = MGC_ERROR_ERR;
    }
    else if(wCsrVal & MGC_M_CSR0_H_NAKTIMEOUT)
    {
	printk(KERN_INFO "%s: NAK timeout\n", __FUNCTION__);
	pThis->bError = TRUE;
	pThis->bErrorCode = MGC_ERROR_NAK;
	/* use the proper sequence to abort the transfer */
	if(wCsrVal & MGC_M_CSR0_H_REQPKT)
	{
	    wCsrVal &= ~MGC_M_CSR0_H_REQPKT;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsrVal);
	    wCsrVal &= ~MGC_M_CSR0_H_NAKTIMEOUT;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsrVal);
	}
	else
	{
	    wCsrVal |= MGC_M_CSR0_FLUSHFIFO;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsrVal);
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsrVal);
	    wCsrVal &= ~MGC_M_CSR0_H_NAKTIMEOUT;
	    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, wCsrVal);
	}
    }

    if(pThis->bError)
    {
        /* clear error and return */
	MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 0);
	return;
    }
    else if(wCsrVal & MGC_M_CSR0_RXPKTRDY)
    {
	if(wCount || pThis->bIsIn)
	{
	    /* rx, so unload FIFO */
	    printk(KERN_INFO "%s: unloading %d bytes from FIFO\n", __FUNCTION__, wCount);
	    MGC_HdrcUnloadFifo(pBase, 0, wCount, &(pThis->aDescriptor[pThis->bRxOffset]));
	    pThis->bRxOffset += wCount;
	    if(wCount > pThis->bRxRemain)
	    {
		pThis->bRxRemain = 0;
	    }
	    else
	    {
		pThis->bRxRemain -= wCount;
	    }
	    pThis->bEnd0Stage = MGC_END0_IN;
	}
	MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, 0);
    }

    if(!pThis->bSingleStep)
    {
	MGC_TestProcNextStep(pThis);
    }
}

/* Read the registers and generate the report.
 */
static int mgc_testproc_report(MGC_TestProcData* pThis) {
    char aBuffer[128];
    int code;
    char* msg;
    char *page=pThis->ProcData.aBuffer;
    uint8_t* pRegs = pThis->pCd->pRegs;

    pThis->ProcData.nSize=0;
    pThis->ProcData.aBuffer[0]=0;

    code=sprintf(page, "Bus=%02x, Test: %s (%02x) / Address %02x, Speed %d, Hub %02x (port %d)\n", 
	pThis->pCd->pBus->busnum, MGC_aTestName[pThis->bTest], 
	MGC_Read8(pRegs, MGC_O_HDRC_TESTMODE), pThis->bPeerAddress, pThis->bPeerSpeed,
	pThis->bPeerHubAddr, pThis->bPeerHubPort);
    if ( code<0 ) {
    	printk(KERN_INFO "%s:%d: A problem generating the report\n",
		__FUNCTION__, __LINE__);
	return pThis->ProcData.nSize;
    } else {
	pThis->ProcData.nSize += code;	
    }

#ifdef MUSB_OTG
    code=sprintf(aBuffer, "OTG state=%s%s\n", 
	MGC_aOtgStateNameName[pThis->pCd->OtgMachine.bState],
	MGC_aOtgError[pThis->pCd->bOtgError]);
    if ( code<0 ) {
	printk(KERN_INFO "%s:%d: A problem generating the report\n",
	    __FUNCTION__, __LINE__);
	return pThis->ProcData.nSize;
    } else {
	strcat(page, aBuffer);
	pThis->ProcData.nSize += code;	
    }
#endif

    if(pThis->bSingleStep)
    {
	code=sprintf(page, "Single-step mode, phase=%s\n", 
	    MGC_aPhaseName[pThis->bEnd0Stage]);
	if ( code<0 ) {
	    printk(KERN_INFO "%s:%d: A problem generating the report\n",
		__FUNCTION__, __LINE__);
	    return pThis->ProcData.nSize;
	} else {
	    pThis->ProcData.nSize += code;	
	}
    }

    if(pThis->bLoop)
    {
	strcat(page, "Loop mode\n");
	pThis->ProcData.nSize += strlen("Loop mode\n");	
    }

    if(pThis->bIsDisconnected)
    {
	strcat(page, "Disconnect detected\n");
	pThis->ProcData.nSize += strlen("Disconnect detected\n");	
    }

    if(pThis->bError)
    {
	strcat(page, "Error detected: ");
	pThis->ProcData.nSize += strlen("Error detected: ");
	switch(pThis->bErrorCode)
	{
	case MGC_ERROR_STALL:
	    msg = "STALL\n";
	    break;
	case MGC_ERROR_ERR:
	    msg = "Rx Error\n";
	    break;
	case MGC_ERROR_NAK:
	    msg = "NAK limit\n";
	    break;
	default:
	    msg = "Unknown\n";
	}
	strcat(page, msg);
	pThis->ProcData.nSize += strlen(msg);
    }

    strcat(page, "Device Descriptor:\n");
    pThis->ProcData.nSize += strlen("Device Descriptor:\n");	

    code = snprintf(aBuffer, 128, "\tbLength/Type=%d/%d\n", 
	pThis->aDescriptor[0], pThis->aDescriptor[1]);
    if ( code<0 ) {
	printk(KERN_INFO "%s:%d: A problem generating the report\n",
	    __FUNCTION__, __LINE__);
	return pThis->ProcData.nSize;
    } else {
	pThis->ProcData.nSize += code;	
	strcat(page, aBuffer);	    
    }
    code = snprintf(aBuffer, 128, "\tbcdUSB/bMaxPacketSize0=%04x/%02x\n", 
	(pThis->aDescriptor[3] << 8) | pThis->aDescriptor[2],
	pThis->aDescriptor[7]);
    if ( code<0 ) {
	printk(KERN_INFO "%s:%d: A problem generating the report\n",
	    __FUNCTION__, __LINE__);
	return pThis->ProcData.nSize;
    } else {
	pThis->ProcData.nSize += code;	
	strcat(page, aBuffer);	    
    }
    code = snprintf(aBuffer, 128, "\tbDeviceClass/Subclass/Protocol=%d/%d/%d\n", 
	pThis->aDescriptor[4], pThis->aDescriptor[5], pThis->aDescriptor[6]);
    if ( code<0 ) {
	printk(KERN_INFO "%s:%d: A problem generating the report\n",
	    __FUNCTION__, __LINE__);
	return pThis->ProcData.nSize;
    } else {
	pThis->ProcData.nSize += code;	
	strcat(page, aBuffer);	    
    }
    code = snprintf(aBuffer, 128, "\tVID=%04x/PID=%04x/Version=%04x\n", 
	(pThis->aDescriptor[9] << 8) | pThis->aDescriptor[8],
	(pThis->aDescriptor[11] << 8) | pThis->aDescriptor[10],
	(pThis->aDescriptor[13] << 8) | pThis->aDescriptor[12]);
    if ( code<0 ) {
	printk(KERN_INFO "%s:%d: A problem generating the report\n",
	    __FUNCTION__, __LINE__);
	return pThis->ProcData.nSize;
    } else {
	pThis->ProcData.nSize += code;	
	strcat(page, aBuffer);	    
    }
    code = snprintf(aBuffer, 128, "\tiManufacturer=%d/Product=%d/Serial=%d\n", 
	pThis->aDescriptor[14], pThis->aDescriptor[15], pThis->aDescriptor[16]);
    if ( code<0 ) {
	printk(KERN_INFO "%s:%d: A problem generating the report\n",
	    __FUNCTION__, __LINE__);
	return pThis->ProcData.nSize;
    } else {
	pThis->ProcData.nSize += code;	
	strcat(page, aBuffer);	    
    }
    code = snprintf(aBuffer, 128, "\tbNumConfigurations=%d\n", pThis->aDescriptor[17]);
    if ( code<0 ) {
	printk(KERN_INFO "%s:%d: A problem generating the report\n",
	    __FUNCTION__, __LINE__);
	return pThis->ProcData.nSize;
    } else {
	pThis->ProcData.nSize += code;	
	strcat(page, aBuffer);	    
    }

    return pThis->ProcData.nSize;
}

/* Write to ProcFS
 *
 * C simulate connect
 * c simulate disconnect
 * L loop mode
 * l single-operation mode
 * B enumerate bus
 * Axx set peer address to xx (hex); 00 means no peer
 * TN no test mode
 * TJ set TEST_J
 * TK set TEST_K
 * TE set TEST_SE0_NAK
 * TP set TEST_PACKET
 * S suspend bus
 * R resume bus
 * E reset bus
 * TDJ set TEST_J on device
 * TDK set TEST_K on device
 * TDE set TEST_SE0_NAK on device
 * TDP set TEST_PACKET on device
 * TDF set TEST_FORCE_ENABLE on device (hubs only)
 * TAxx store address xx (hex) for TSA below
 * TGD GET_DESCRIPTOR(DEVICE) on peer
 * TSA SET_ADDRESS(xx) on peer
 * TSL set peer as low-speed
 * TSU set peer as full-speed
 * TSH set peer as high-speed
 * TSBxx set peer parent hub address
 * TSPxx set peer parent hub port
 * TWE enable wakeup on peer
 * TWD disable wakeup on peer
 * TSF single-step SET_FEATURE
 * TSG single-step GET_DESCRIPTOR(DEVICE)
 * N next step in single-step operations
 */
static int MGC_TestProcWrite(struct file *file, const char *buffer,
			     unsigned long count, void *data)
{
    char c;
    char cmd[32];
    uint8_t bReg;
    uint8_t bIndex;
    uint8_t bValue;
    MGC_TestProcData* pThis=(MGC_TestProcData*)data;
    uint8_t* pRegs=pThis->pCd->pRegs;
    uint8_t* pBase=pRegs;

    MGC_HdrcSetDisconnectListener(pThis->pCd, MGC_TestProcDisconnectListener,
	pThis);

    (void)copy_from_user(&cmd, buffer, min((size_t)count, sizeof(cmd)));
    bIndex = 0;
    while(bIndex < count)
    {
	c = cmd[bIndex];
	switch(c)
	{
#if MUSB_DEBUG > 0
	case 'P': MGC_HdrcDumpRegs(pRegs, 0, 0); return count;
#endif
	case 'r':
		switch(cmd[bIndex+1]) {
			case 'b':
			printk(KERN_INFO "[%08x@%04x] = 0x%04x] \n", pRegs, 
				simple_strtol(&cmd[3], NULL, 16),
				MGC_Read8(pRegs, simple_strtol(&cmd[3], NULL, 16)));
			return count;
			case 'w':
			printk(KERN_INFO "[%08x@%04x] = 0x%04x] \n", pRegs, 
				simple_strtol(&cmd[3], NULL, 16),
				MGC_Read16(pRegs, simple_strtol(&cmd[3], NULL, 16)));
			return count;
		}
		return count;

	case 'w': 
		switch(cmd[bIndex+1]) {
			case 'b':
			MGC_Write8(pRegs, 
				simple_strtol(&cmd[3], NULL, 16),
				simple_strtol(&cmd[8], NULL, 16));
			return count;
			case 'w':
			MGC_Write16(pRegs, 
				simple_strtol(&cmd[3], NULL, 16),
				simple_strtol(&cmd[8], NULL, 16));
			return count;
		}
		return count;

	case 'C':
	  printk(KERN_INFO "simulating connect...\n");
	  mgc_hdrc_service_usb(pThis->pCd, MGC_M_INTR_CONNECT);
	  break;

	case 'c':
	  printk(KERN_INFO "simulating disconnect...\n");
	  mgc_hdrc_service_usb(pThis->pCd, MGC_M_INTR_DISCONNECT);
	  break;

	case 'L':
	    pThis->bLoop = TRUE;
	    bIndex++;
	    break;

	case 'l':
	    pThis->bLoop = FALSE;
	    bIndex++;
	    break;

	case 'B':
	    /* stop & start so Linux hub driver will re-enumerate */
	    if ( pRegs ) {
		MGC_LinuxCd* pCd = pThis->pCd;
		MGC_HdrcSetDefaultEndHandler(pCd, NULL, NULL);
		MGC_HdrcStop(pCd);
		MGC_VirtualHubPortDisconnected(&(pCd->RootHub), 0);
		pCd->pRootDevice = NULL;
		MGC_OtgUpdate(pCd, TRUE, FALSE);
		WAIT_MS(1000);
		MGC_HdrcStart(pCd);
	    }
	    bIndex++;
	    break;

	case 'A':
	    /* parse peer address in hex (uppercase letters only) */
	    bValue = (cmd[bIndex+1] >= 'A') ? (cmd[bIndex+1] - 'A') : (cmd[bIndex+1] - '0');
	    bValue <<= 4;
	    bValue |= (cmd[bIndex+2] >= 'A') ? (cmd[bIndex+2] - 'A') : (cmd[bIndex+2] - '0');
	    pThis->bPeerAddress = bValue;
	    bIndex += 3;
	    break;

	case 'S':
	    /* suspend */
	    if ( pBase ) {
		bReg = MGC_Read8(pBase, MGC_O_HDRC_POWER);
		MGC_Write8(pBase, MGC_O_HDRC_POWER, bReg | MGC_M_POWER_SUSPENDM);
		printk(KERN_INFO "Bus Suspended\n");
		pThis->bIsSuspended = TRUE;
	    }
	    bIndex++;
	    break;

	case 'R':
	    /* resume */
	    if ( pBase ) {
		bReg = MGC_Read8(pBase, MGC_O_HDRC_POWER);
		MGC_Write8(pBase, MGC_O_HDRC_POWER, bReg | MGC_M_POWER_RESUME);
		WAIT_MS(10);
		MGC_Write8(pBase, MGC_O_HDRC_POWER, bReg);
		printk(KERN_INFO "Bus Resumed\n");
		pThis->bIsSuspended = FALSE;
	    }
	    bIndex++;
	    break;

	case 'E':
	    /* reset */
	    if ( pBase ) {
		bReg = MGC_Read8(pBase, MGC_O_HDRC_POWER);
		MGC_Write8(pBase, MGC_O_HDRC_POWER, bReg | MGC_M_POWER_RESET);
		WAIT_MS(50);
		MGC_Write8(pBase, MGC_O_HDRC_POWER, bReg);
		printk(KERN_INFO "Bus Reset\n");
		pThis->bIsSuspended = FALSE;
	    }
	    bIndex++;
	    break;

	case 'N':
	    /* next */
	    if(!pThis->bError)
	    {
		MGC_TestProcNextStep(pThis);
	    }
	    bIndex++;
	    break;

	case 'T':
	    c = cmd[bIndex+1];
	    switch(c)
	    {
	    case 'A':
		/* parse test address in hex (uppercase letters only) */
		bValue = (cmd[bIndex+2] >= 'A') ? (cmd[bIndex+2] - 'A') : (cmd[bIndex+2] - '0');
		bValue <<= 4;
		bValue |= (cmd[bIndex+3] >= 'A') ? (cmd[bIndex+3] - 'A') : (cmd[bIndex+3] - '0');
		pThis->bTestAddress = bValue;
		bIndex += 4;
		break;
	    case 'D':
		/* device test modes */
		switch(cmd[bIndex+2])
		{
		case 'J':
		    pThis->bTest = MGC_TEST_J;
		    pThis->bLoop = FALSE;
		    pThis->bSingleStep = FALSE;
		    pThis->bError = FALSE;
		    pThis->pSetup = MGC_aSetTestMode;
		    MGC_aSetTestMode[5] = 1;
		    pThis->bRxSize = 0;
		    MGC_TestProcStartSetup(pThis);
		    break;
		case 'K':
		    pThis->bTest = MGC_TEST_K;
		    pThis->bLoop = FALSE;
		    pThis->bSingleStep = FALSE;
		    pThis->bError = FALSE;
		    pThis->pSetup = MGC_aSetTestMode;
		    MGC_aSetTestMode[5] = 2;
		    pThis->bRxSize = 0;
		    MGC_TestProcStartSetup(pThis);
		    break;
		case 'E':
		    pThis->bTest = MGC_TEST_SE0_NAK;
		    pThis->bLoop = FALSE;
		    pThis->bSingleStep = FALSE;
		    pThis->bError = FALSE;
		    pThis->pSetup = MGC_aSetTestMode;
		    MGC_aSetTestMode[5] = 3;
		    pThis->bRxSize = 0;
		    MGC_TestProcStartSetup(pThis);
		    break;
		case 'P':
		    pThis->bTest = MGC_TEST_PACKET;
		    pThis->bLoop = FALSE;
		    pThis->bSingleStep = FALSE;
		    pThis->bError = FALSE;
		    pThis->pSetup = MGC_aSetTestMode;
		    MGC_aSetTestMode[5] = 4;
		    pThis->bRxSize = 0;
		    MGC_TestProcStartSetup(pThis);
		    break;
		case 'F':
		    pThis->bTest = MGC_TEST_NONE;
		    pThis->bLoop = FALSE;
		    pThis->bSingleStep = FALSE;
		    pThis->bError = FALSE;
		    pThis->pSetup = MGC_aSetTestMode;
		    MGC_aSetTestMode[5] = 5;
		    pThis->bRxSize = 0;
		    MGC_TestProcStartSetup(pThis);
		    break;
		}
		bIndex += 3;
		break;
	    case 'N':
		/* no test */
		if(pRegs)
		{
		    MGC_Write8(pRegs, MGC_O_HDRC_TESTMODE, 0);
		    pThis->bTest = MGC_TEST_NONE;
		    pThis->bLoop = FALSE;
		    pThis->bSingleStep = FALSE;
		    pThis->bError = FALSE;
		    MGC_HdrcSetDefaultEndHandler(pThis->pCd, NULL, NULL);
		}
		bIndex += 2;
		break;
	    case 'J':
		/* set TEST_J */
		if(pRegs)
		{
		    MGC_Write8(pRegs, MGC_O_HDRC_TESTMODE, MGC_M_TEST_J);
		    pThis->bTest = MGC_TEST_J;
		}
		bIndex += 2;
		break;
	    case 'K':
		/* set TEST_K */
		if(pRegs)
		{
		    MGC_Write8(pRegs, MGC_O_HDRC_TESTMODE, MGC_M_TEST_K);
		    pThis->bTest = MGC_TEST_K;
		}
		bIndex += 2;
		break;
	    case 'E':
		/* set TEST_SE0_NAK */
		if(pRegs)
		{
		    MGC_Write8(pRegs, MGC_O_HDRC_TESTMODE, MGC_M_TEST_SE0_NAK);
		    pThis->bTest = MGC_TEST_SE0_NAK;
		}
		bIndex += 2;
		break;
	    case 'P':
		/* TEST_PACKET */
		if(pRegs)
		{
		    MGC_SelectEnd(pRegs, 0);
#ifdef MUSB_DMA2
		    MGC_HdrcLoadFifo(pThis, pRegs, 0, sizeof(MGC_aTestPacket), MGC_aTestPacket);
#else
		    MGC_HdrcLoadFifo(pRegs, 0, sizeof(MGC_aTestPacket), MGC_aTestPacket);
#endif
			/* despite explicit instruction, we still must kick-start */
		    MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_TXPKTRDY);
		    MGC_Write8(pRegs, MGC_O_HDRC_TESTMODE, MGC_M_TEST_PACKET);
		    pThis->bTest = MGC_TEST_PACKET;
		}
		bIndex += 2;
		break;
	    case 'G':
		if('D' == cmd[bIndex+2])
		{
		    /* GET_DESC(DEVICE) */
		    pThis->bTest = MGC_TEST_GET_DESC;
		    pThis->bSingleStep = FALSE;
		    pThis->pSetup = MGC_aGetDeviceDescriptor;
		    pThis->bRxSize = 18;
		    MGC_TestProcStartSetup(pThis);
		}
		bIndex += 3;
		break;
	    case 'W':
		if('E' == cmd[bIndex+2])
		{
		    /* enable remote wake */
		    pThis->bTest = MGC_TEST_ENABLE_WAKE;
		    pThis->bSingleStep = FALSE;
		    pThis->pSetup = MGC_aSetRemoteWake;
		    pThis->bRxSize = 0;
		    MGC_TestProcStartSetup(pThis);
		}
		else if('D' == cmd[bIndex+2])
		{
		    /* disable remote wake */
		    pThis->bTest = MGC_TEST_DISABLE_WAKE;
		    pThis->bSingleStep = FALSE;
		    pThis->pSetup = MGC_aClearRemoteWake;
		    pThis->bRxSize = 0;
		    MGC_TestProcStartSetup(pThis);
		}
		bIndex += 3;
		break;
	    case 'S':
		switch(cmd[bIndex+2])
		{
		case 'L':
		    pThis->bPeerSpeed = USB_SPEED_LOW;
		    bIndex += 3;
		    break;
		case 'U':
		    pThis->bPeerSpeed = USB_SPEED_FULL;
		    bIndex += 3;
		    break;
		case 'H':
		    pThis->bPeerSpeed = USB_SPEED_HIGH;
		    bIndex += 3;
		    break;
		case 'B':
		    c = cmd[bIndex+3];
		    bValue = (c >= 'A') ? (c - 'A') : (c - '0');
		    bValue <<= 4;
		    c = cmd[bIndex+4];
		    bValue |= (c >= 'A') ? (c - 'A') : (c - '0');
		    pThis->bPeerHubAddr = bValue;
		    bIndex += 5;
		    break;
		case 'P':
		    c = cmd[bIndex+3];
		    bValue = (c >= 'A') ? (c - 'A') : (c - '0');
		    bValue <<= 4;
		    c = cmd[bIndex+4];
		    bValue |= (c >= 'A') ? (c - 'A') : (c - '0');
		    pThis->bPeerHubPort = bValue;
		    bIndex += 5;
		    break;
		case 'A':
		    /* SET_ADDRESS */
		    pThis->bTest = MGC_TEST_SET_ADDR;
		    pThis->bSingleStep = FALSE;
		    MGC_aSetDeviceAddress[2] = pThis->bPeerAddress;
		    pThis->pSetup = MGC_aSetDeviceAddress;
		    pThis->bRxSize = 0;
		    MGC_TestProcStartSetup(pThis);
		    bIndex += 3;
		    break;
		case 'F':
		    /* single-step SET_FEATURE() */
		    pThis->bTest = MGC_TEST_STEP_SET_FEATURE;
		    pThis->bSingleStep = TRUE;
		    pThis->pSetup = MGC_aSetFeature;
		    pThis->bRxSize = 0;
		    MGC_TestProcStartSetup(pThis);
		    bIndex += 3;
		    break;
		case 'G':
		    /* single-step GET_DESC(DEVICE) */
		    pThis->bTest = MGC_TEST_STEP_GET_DESC;
		    pThis->bSingleStep = TRUE;
		    pThis->pSetup = MGC_aGetDeviceDescriptor;
		    pThis->bRxSize = 18;
		    MGC_TestProcStartSetup(pThis);
		    bIndex += 3;
		    break;
		}
		break;
	    }
	    break;

	case '\n':
	    bIndex++;
	    break;

	default:
	    if(!isalnum(cmd[bIndex]))
	    {
		bIndex++;
	    }
	    else
	    {
		DBG(2, "Command not implemented\n");
		bIndex = count;
	    }
	}
    }

    /* MOD_DEC_USE_COUNT; */
    return count;
} 

/*
 *
 *
 *
 *
 */
static int MGC_TestProcRead(char *page, char **start, 
	off_t off, int count, int *eof, void *data) 
{
	int rc=0;	
	off_t len;
	char *buffer;
	unsigned long flags;
	MGC_TestProcData* pThis=(MGC_TestProcData*)data;

	/* MOD_INC_USE_COUNT; */

	spin_lock_irqsave(&pThis->Lock, flags);
	buffer=pThis->ProcData.aBuffer;
	len=pThis->ProcData.nSize;

	if ( !*buffer || !off) {	
	    len=mgc_testproc_report(pThis);
	}

	if ( off<len ) {
	    int i=0, togo=len-off;
	    char *s=&buffer[off],*d=page;
	    	    
	    if ( togo>count ) {
	    	togo=count;
	    }
	    
	    while ( i++<togo ) {
		    *d++=*s++;
	    }

	    rc=togo;	    
	} else {
	    *buffer=0;
	    *eof=1;	
	}
	
	/* MOD_DEC_USE_COUNT; */

	spin_unlock_irqrestore(&pThis->Lock, flags);
	return rc;
} 


/**
 * Driver for test device
 */
#ifdef MUSB_V26
STATIC int MGC_TestModeDeviceProbe(struct usb_interface *intf, 
	const struct usb_device_id *id)
{
    struct usb_device *dev = interface_to_usbdev(intf);
#else
STATIC void* MGC_TestModeDeviceProbe(struct usb_device *dev, unsigned int i,
	const struct usb_device_id *id)
{
#endif
    uint8_t bReg;
    struct usb_device* pParent;
    uint8_t bHubAddr = 0;
    uint8_t bHubPort = 0;
    MGC_TestProcData* pThis = MGC_pTestProcData;
    uint8_t* pRegs=pThis->pCd->pRegs;
    uint8_t* pBase=pRegs;
#ifdef HAS_USB_TT_MULTI
   uint8_t bIsMulti = FALSE;
#endif

   pThis->bSingleStep = FALSE;
   pThis->bNoStatusPhase = FALSE;

    /* NOTE: there is always a parent due to the virtual root hub */
    /* parent hub address */
    pParent = dev->parent;
    bHubAddr = (uint8_t)pParent->devnum;
    if(bHubAddr == pThis->pCd->RootHub.bAddress)
    {
        /* but not if parent is our virtual root hub */
	bHubAddr = 0;
    }
    /* parent port */
    /* if tt pointer, use its info */
    if(dev->tt)
    {
	bHubPort = (uint8_t)dev->ttport;
#ifdef HAS_USB_TT_MULTI
	bIsMulti = (uint8_t)dev->tt->multi;
#endif
    }

    pThis->bPeerSpeed = dev->speed;
    pThis->bPeerAddress = dev->devnum;
    pThis->bPeerHubAddr = bHubAddr;
    pThis->bPeerHubPort = bHubPort;

    switch( id->idProduct )
    {
    case 0x0101:
	/* SE0_NAK */
	MGC_Write8(pRegs, MGC_O_HDRC_TESTMODE, MGC_M_TEST_SE0_NAK);
	pThis->bTest = MGC_TEST_SE0_NAK;
	break;

    case 0x0102:
	/* J */
	MGC_Write8(pRegs, MGC_O_HDRC_TESTMODE, MGC_M_TEST_J);
	pThis->bTest = MGC_TEST_J;
	break;

    case 0x0103:
	/* K */
	MGC_Write8(pRegs, MGC_O_HDRC_TESTMODE, MGC_M_TEST_K);
	pThis->bTest = MGC_TEST_K;
	break;

    case 0x0104:
	/* PACKET */
#ifdef MUSB_DMA2
	MGC_HdrcLoadFifo(pThis, pRegs, 0, sizeof(MGC_aTestPacket), MGC_aTestPacket);
#else
	MGC_HdrcLoadFifo(pRegs, 0, sizeof(MGC_aTestPacket), MGC_aTestPacket);
#endif
	/* despite explicit instruction, we still must kick-start */
	MGC_WriteCsr16(pBase, MGC_O_HDRC_CSR0, 0, MGC_M_CSR0_TXPKTRDY);
	MGC_Write8(pRegs, MGC_O_HDRC_TESTMODE, MGC_M_TEST_PACKET);
	pThis->bTest = MGC_TEST_PACKET;
	break;

    case 0x0105:
	/* FORCE_ENABLE */
	MGC_Write8(pRegs, MGC_O_HDRC_TESTMODE, MGC_M_TEST_FORCE_HS);
	pThis->bTest = MGC_TEST_NONE;
	break;

    case 0x0106:
	/* suspend, wait 20 secs, resume */
	bReg = MGC_Read8(pBase, MGC_O_HDRC_POWER);
	MGC_Write8(pBase, MGC_O_HDRC_POWER, bReg | MGC_M_POWER_SUSPENDM);
	printk(KERN_INFO "Bus Suspended\n");
	pThis->bIsSuspended = TRUE;
	WAIT_MS(20*1000);
	bReg = MGC_Read8(pBase, MGC_O_HDRC_POWER);
	MGC_Write8(pBase, MGC_O_HDRC_POWER, bReg | MGC_M_POWER_RESUME);
	WAIT_MS(10);
	MGC_Write8(pBase, MGC_O_HDRC_POWER, bReg);
	printk(KERN_INFO "Bus Resumed\n");
	pThis->bIsSuspended = FALSE;
	break;

    case 0x0107:
	/* wait 15 secs, perform setup phase of GET_DESC */
	WAIT_MS(15*1000);
	pThis->bTest = MGC_TEST_STEP_GET_DESC;
	pThis->bSingleStep = TRUE;
	pThis->pSetup = MGC_aGetDeviceDescriptor;
	pThis->bRxSize = 18;
	MGC_TestProcStartSetup(pThis);
	break;

    case 0x0108:
	/* perform setup phase of GET_DESC, wait 15 secs, perform IN data phase */
	pThis->bTest = MGC_TEST_STEP_GET_DESC;
	pThis->bSingleStep = TRUE;
	pThis->bNoStatusPhase = TRUE;
	pThis->pSetup = MGC_aGetDeviceDescriptor;
	pThis->bRxSize = 18;
	MGC_TestProcStartSetup(pThis);
	WAIT_MS(15*1000);
	pThis->bSingleStep = FALSE;
	MGC_TestProcNextStep(pThis);
	break;
    }
    return 0;
}

#ifdef MUSB_V26
STATIC void MGC_TestModeDeviceDisconnect(struct usb_interface *intf)
#else
STATIC void MGC_TestModeDeviceDisconnect(struct usb_device *dev, void *ptr)
#endif
{
    /* nothing to do */
}

/**
 * TODO: 2.4 and 2.6 create the pseudo-device in 2 different points
 */
void MGC_LinuxRemoveTestProcFs(MGC_TestProcData* pThis) {    
    remove_proc_entry(pThis->pProcEntry->name, NULL);
    usb_deregister(&MGC_TestModeDeviceDriver);
}

void MGC_LinuxDeleteTestProcFs(char *name, MGC_LinuxCd* pIcd)
{
    MGC_LinuxRemoveTestProcFs(MGC_pTestProcData);
}

/**
 * TODO: 2.4 and 2.6 create the pseudo-device in 2 different points
 */
struct proc_dir_entry* MGC_LinuxCreateTestProcFs(char *name, MGC_LinuxCd* pIcd) {
    MGC_TestProcData* pThis;
    char realname[64];

    if ( !name ) {
    	name=pIcd->aName;
    }
    strcpy(realname, "test");
    strcat(realname, name);

    KMALLOC(pThis, sizeof(MGC_TestProcData), GFP_KERNEL);    
    if(!pThis)
    {
        printk(KERN_ERR "%s: kmalloc testproc instance data failed\n",
	       __FUNCTION__);
	return NULL;
    }
    MGC_pTestProcData = pThis;
    memset (pThis, 0, sizeof(MGC_TestProcData));
    pThis->pCd = pIcd;

    usb_register(&MGC_TestModeDeviceDriver);

    pThis->pProcEntry=create_proc_entry(realname, 
	S_IFREG | S_IRUGO | S_IWUSR, NULL);
    if( pThis->pProcEntry )
    {
	pThis->pProcEntry->data = pThis;
#ifdef MUSB_V26
	pThis->pProcEntry->owner=THIS_MODULE;
#endif
	
	pThis->pProcEntry->read_proc = MGC_TestProcRead;
	pThis->pProcEntry->write_proc = MGC_TestProcWrite;

	pThis->pProcEntry->size = 0;

    	dbg("Registered /proc/%s\n", realname);
    } else {
        dbg ("Cannot create a valid proc file entry");    
    }

    spin_lock_init(&pThis->Lock);
    return pThis->pProcEntry;  
}


