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
 
 * The code managing procfs.
 * $Revision: 1.1.1.1 $
 */

#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include "musbdefs.h"
#include "plat_arc.h"
#include "debug.h"

#ifdef CONFIG_PROC_FS

/* Read the registers and generate the report.
 */
static int mgc_proc_report(MGC_LinuxCd* pThis) {
    char* pProto;
    uint8_t bEnd;
    char aBuffer[128];
    int debug=-1, code;

    const uint8_t* pBase=pThis->pRegs;
    char *page=pThis->MGC_ProcData.aBuffer;

#if MUSB_DEBUG>0 
    debug=MGC_DebugLevel;
#endif    

    pThis->MGC_ProcData.nSize=0;
    pThis->MGC_ProcData.aBuffer[0]=0;

    code=sprintf(page, 
    	"%sDRC, Mode=%s [debug=%d][gadget=%s][otg=%s][eps=%d] (Power=%02x, DevCtl=%02x)\n",
	( pThis->bIsMultipoint ? "MH" : "H"),
	MUSB_MODE(pThis),
	debug,	
#ifdef MUSB_GADGET
"yes"
#else
"no"
#endif
	,
#ifdef MUSB_OTG
"yes"
#else
"no"
#endif    	
	, pThis->bEndCount,
	MGC_Read8(pBase, MGC_O_HDRC_POWER),
	MGC_Read8(pBase, MGC_O_HDRC_DEVCTL));
    if ( code<0 ) {
    	printk(KERN_INFO "%s:%d: A problem generating the report\n",
		__FUNCTION__, __LINE__);
	return pThis->MGC_ProcData.nSize;
    } else {
	    pThis->MGC_ProcData.nSize+=code;	
    }

	
    for(bEnd = 0; bEnd < pThis->bEndCount; bEnd++)
    {

    	MGC_LinuxLocalEnd* pEnd = &(pThis->aLocalEnd[bEnd]);
	spin_lock(&pEnd->Lock);
	
	if(pThis->bIsDevice)
	{
	    switch(pEnd->bTrafficType)
	    {
	    case USB_ENDPOINT_XFER_CONTROL:
		pProto = "Ctrl";
		break;
	    case USB_ENDPOINT_XFER_ISOC:
		pProto = "Isoc";
		break;
	    case USB_ENDPOINT_XFER_BULK:
		pProto = "Bulk";
		break;
	    case USB_ENDPOINT_XFER_INT:
		pProto = "Intr";
		break;
	    default:
		pProto = "Err ";
	    }
	} else {
	    switch(pEnd->bTrafficType)
	    {
	    case PIPE_ISOCHRONOUS:
		pProto = "Isoc";
		break;
	    case PIPE_INTERRUPT:
		pProto = "Intr";
		break;
	    case PIPE_CONTROL:
		pProto = "Ctrl";
		break;
	    case PIPE_BULK:
		pProto = "Bulk";
		break;
	    default:
		pProto = "Err ";
	    }
	}

	code=snprintf(aBuffer, 128, "End-%01x: %s, %s, %s, proto=%s, pkt size=%04x, address=%02x, end=%02x\n", 
	    bEnd, 
	    (pEnd->pUrb ? "Busy" : "Idle"),
	    ( list_empty(&(pEnd->urb_list)) ? "Q Empty" : " Q Full" ),
	    ( pEnd->bIsTx ? "Tx" : "Rx" ),
	    pProto, 
	    pEnd->wPacketSize, 
	    pEnd->bAddress, 
	    pEnd->bEnd);
	if ( code<0 ) {
    	   printk(KERN_INFO "%s:%d: A problem generating the report\n",
		   __FUNCTION__, __LINE__);
	    return pThis->MGC_ProcData.nSize;
	} else {
		pThis->MGC_ProcData.nSize+=code;	
		strcat(page, aBuffer);	    
	}

	code = snprintf(aBuffer, 128, 
	    "  %10ld bytes Rx in %10ld pkts; %10ld errs, %10ld overruns\n",
	    pEnd->dwTotalRxBytes, 
	    pEnd->dwTotalRxPackets,
	    pEnd->dwErrorRxPackets, pEnd->dwMissedRxPackets);
	if ( code<0 ) {
    	   printk(KERN_INFO "%s:%d: A problem generating the report\n",
		   __FUNCTION__, __LINE__);
	    return pThis->MGC_ProcData.nSize;
	} else {
		pThis->MGC_ProcData.nSize+=code;	
		strcat(page, aBuffer);	    
	}

	code= snprintf(aBuffer, 128,
	    "  %10ld bytes Tx in %10ld pkts; %10ld errs, %10ld underruns\n",
	    pEnd->dwTotalTxBytes, 
	    pEnd->dwTotalTxPackets,
	    pEnd->dwErrorTxPackets, pEnd->dwMissedTxPackets);
	if ( code<0 ) {
    	   printk(KERN_INFO "%s:%d: A problem generating the report\n",
		   __FUNCTION__, __LINE__);
	    return pThis->MGC_ProcData.nSize;
	} else {
		pThis->MGC_ProcData.nSize+=code;	
		strcat(page, aBuffer);	    
	}	

	spin_unlock(&pEnd->Lock);
    }
    
    return pThis->MGC_ProcData.nSize;
}

/* Write to ProcFS
 *
 * R resume bus
 * S start session
 * H request host mode
 * D<num> set/query the debug level
 * Z zap
 */
static int MGC_ProcWrite(struct file *file, const char *buffer,
                           unsigned long count, void *data)
{
    char cmd;
    uint8_t bReg;    
    uint8_t* pBase=((MGC_LinuxCd*)data)->pRegs;

    (void)copy_from_user(&cmd, buffer, 1);
    switch(cmd)
    {
    case 'R':
    	if ( pBase ) {
        	bReg = MGC_Read8(pBase, MGC_O_HDRC_POWER);
		MGC_Write8(pBase, MGC_O_HDRC_POWER, bReg | MGC_M_POWER_RESUME);
		WAIT_MS(10);
		MGC_Write8(pBase, MGC_O_HDRC_POWER, bReg);
		DEBUG_CODE(2, printk(KERN_INFO "%s: Power Resumed\n", \
		    __FUNCTION__); )
	}
	break;
    case 'S':
    	if ( pBase ) {
        	bReg = MGC_Read8(pBase, MGC_O_HDRC_DEVCTL);
		bReg |= MGC_M_DEVCTL_SESSION;
		MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, bReg);
		DEBUG_CODE(2, printk(KERN_INFO "%s: Session Mode\n", \
		    __FUNCTION__); )
	}
	break;
    case 'H':
    	if ( pBase ) {
        	bReg = MGC_Read8(pBase, MGC_O_HDRC_DEVCTL);
		bReg |= MGC_M_DEVCTL_HR;
		MGC_Write8(pBase, MGC_O_HDRC_DEVCTL, bReg);
		DEBUG_CODE(2, printk(KERN_INFO "%s: Host Mode\n", \
		    __FUNCTION__); )
	}
	break;
	
#if (MUSB_DEBUG>0)	
    case 'Z':
    	if ( pBase ) {
		MGC_LinuxCd* pThis=data;
	
		MGC_HdrcStop(pThis);
		MUSB_ERR_MODE(pThis);
		MGC_VirtualHubPortDisconnected(&(pThis->RootHub), 0);
		pThis->pRootDevice = NULL;
		MGC_OtgUpdate(pThis, TRUE, FALSE);

		DEBUG_CODE(2, printk(KERN_INFO "%s: Stopped\n", \
		    __FUNCTION__); )
		    
		WAIT_MS(1000);
		
		MGC_HdrcStart( pThis );
		DEBUG_CODE(2, printk(KERN_INFO "%s: Started\n", \
		    __FUNCTION__); )
	}
	break;
		
    case 'D': {		
	    if ( count>1 ) {
		    char digits[8], *p=digits;
	    	    int i=0, level=0, sign=1, len=min((int)count-1, 8);
		    
		    (void)copy_from_user(&digits, &buffer[1], len);

		    /* optional sign */
		    if ( *p=='-' ) {
			len-=1; sign=-sign; p++;
		    }
		    
		    /* read it */
		    while ( i++<len && *p>'0' && *p<'9') {
		    	level=level*10 + (*p++ -'0');
		    }		    
		    
		    MGC_DebugLevel=sign*level;		    
		    printk(KERN_INFO "%s: MGC_DebugLevel=%d\n", \
		    	__FUNCTION__, MGC_DebugLevel); 
            } else {
		printk(KERN_INFO "%s: MGC_DebugLevel=%d\n", 
			__FUNCTION__, MGC_DebugLevel); 
	    	/* & dump the status to syslog */
	    }
		 
	} 
	break;
#endif
	default:
	    DEBUG_CODE(2, printk(KERN_INFO "%s: Command not implemented\n", \
		__FUNCTION__ ); )    
	break;
    }

    return count;
} 

/*
 *
 */
static int MGC_ProcRead(char *page, char **start, 
	off_t off, int count, int *eof, void *data) 
{
	int rc=0;	
	off_t len;
	char *buffer;
	unsigned long flags;
        MGC_LinuxCd* pThis=(MGC_LinuxCd*)data;

	spin_lock_irqsave(&pThis->Lock, flags);
	buffer=pThis->MGC_ProcData.aBuffer;
	len=pThis->MGC_ProcData.nSize;

	if ( !*buffer || !off) {	
	    len=mgc_proc_report(pThis);
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

	spin_unlock_irqrestore(&pThis->Lock, flags);
	return rc;
} 

/**
 * TODO: 2.4 and 2.6 create the pseudo-device in 2 different points
 */
void MGC_LinuxDeleteProcFs(MGC_LinuxCd* data) {    
    remove_proc_entry(data->pProcEntry->name, NULL);
}

/**
 * TODO: 2.4 and 2.6 create the pseudo-device in 2 different points
 */
struct proc_dir_entry* MGC_LinuxCreateProcFs(char *name, MGC_LinuxCd* data) {
    if ( !name ) {
    	name=data->aName;
    }

    data->pProcEntry=create_proc_entry(name, 
	S_IFREG | S_IRUGO | S_IWUSR, NULL);
    if( data->pProcEntry )
    {
	data->pProcEntry->data = data;
#ifdef MUSB_V26
	data->pProcEntry->owner=THIS_MODULE;
#endif
	
	data->pProcEntry->read_proc = MGC_ProcRead;
	data->pProcEntry->write_proc = MGC_ProcWrite;

	data->pProcEntry->size = 0;

    	dbg("Registered /proc/%s\n", name);
    } else {
        dbg ("Cannot create a valid proc file entry");    
    }

    return data->pProcEntry;  
}

#endif	/* CONFIG_PROC_FS */

