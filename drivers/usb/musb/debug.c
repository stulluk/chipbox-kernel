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
#include <linux/list.h>
#include <linux/completion.h>
#include <linux/interrupt.h>

#include "debug.h"

#define IPRINTF(_f, _m) printk(KERN_INFO "%s"_f, indent, _m)
#define mgc_isspace(c) (c==' ' || c=='\t')
#define LABEL KERN_INFO "dump: "

int MGC_DebugLevel=MGC_DEBUG;

static void __dump_struct(char *gap, char *var, char *name, void *ptr)
{
        char indent[64], *s=name, *d=indent;

        if ( gap ) {
                while ( *gap ) {
                        *d++=*gap++;
                } /* copy gap */
        }

        while ( mgc_isspace(*s) ) { /* copy the tabs */
                *d++=*s++;
        }
        *d=0;

        printk(KERN_INFO "%s%p:struct %s* %s={\n", indent, ptr, name, var);
        if ( !ptr ) {
                printk(KERN_INFO "}\n");
                return;
        }

        if ( strcmp("completion", name)==0 ) {
                const struct completion* tgt=(struct completion*)ptr;
                IPRINTF(" .done=%u\n", tgt->done);
                __dump_struct(indent, "tgt->wait", " wait_queue_head_t",
                              (void*)&tgt->wait);
        } else if ( strcmp("wait_queue_head_t", name)==0 ) {
        } else if ( strcmp("list_head", name)==0 ) {
                const struct list_head *head=(struct list_head*)ptr;
                IPRINTF(" .next=%p\n", head->next);
                IPRINTF(" .prev=%p\n", head->prev);
        }

        printk(KERN_INFO "}\n");
}

void dump_struct(char *var, char *name, void *ptr)
{
        __dump_struct(NULL, var, name, ptr);
}

void dump_urb (struct urb *purb)
{
        printk (LABEL "urb                   :%p\n", purb);
        printk (LABEL "dev                   :%p\n", purb->dev);
        printk (LABEL "pipe                  :%08X\n", purb->pipe);
        printk (LABEL "status                :%d\n", purb->status);
        printk (LABEL "transfer_flags        :%08X\n", purb->transfer_flags);
        printk (LABEL "transfer_buffer       :%p\n", purb->transfer_buffer);
        printk (LABEL "transfer_buffer_length:%d\n", purb->transfer_buffer_length);
        printk (LABEL "actual_length         :%d\n", purb->actual_length);
        printk (LABEL "setup_packet          :%p\n", purb->setup_packet);
        printk (LABEL "start_frame           :%d\n", purb->start_frame);
        printk (LABEL "number_of_packets     :%d\n", purb->number_of_packets);
        printk (LABEL "interval              :%d\n", purb->interval);
        printk (LABEL "error_count           :%d\n", purb->error_count);
        printk (LABEL "context               :%p\n", purb->context);
        printk (LABEL "complete              :%p\n", purb->complete);
}

/* Decode CSR0 value to a string. Not reentrant
 */
char *decode_csr0(uint16_t csr0)
{
        static char buf[64];

        sprintf(buf, "(%s%s%s%s)",
                csr0&MGC_M_CSR0_TXPKTRDY ? "[TXPKTRDY]":"",
                csr0&MGC_M_CSR0_P_SVDRXPKTRDY ? "[SVDRXPKTRDY]":"",
                csr0&MGC_M_CSR0_P_SENDSTALL ? "[stalled]":"",
                csr0&MGC_M_CSR0_P_DATAEND ? "[dataend]":"");

        return buf;
}

/* Decode a value to binary.
 */
char *decode_bits(uint16_t value)
{
        int i=0;
        static char buf[64];

        for (; i<16;i++) {
                buf[15-i]=(value&(1<<i))?'1':'0';
        }

        return buf;
}

/* Decode TXCSR register.
 */
char *decode_txcsr(uint16_t txcsr)
{
        static char buf[256];

        sprintf(buf, "%s (%s%s%s%s)",
                decode_bits(txcsr),
                txcsr&MGC_M_TXCSR_TXPKTRDY ? "[TXPKTRDY]":"",
                txcsr&MGC_M_TXCSR_AUTOSET ? "[MGC_M_TXCSR_AUTOSET]":"",
                txcsr&MGC_M_TXCSR_DMAENAB ? "[MGC_M_TXCSR_DMAENAB]":"",
                txcsr&MGC_M_TXCSR_DMAMODE ? "[MGC_M_TXCSR_DMAMODE]":"");

        return buf;
}

/*
 */
char *decode_devctl(uint16_t devctl)
{
        return (devctl&MGC_M_DEVCTL_HM)?"host":"function";
}

/*
 */
char *decode_ep0stage(uint8_t stage)
{
        static char buff[64];
        uint8_t stallbit=stage&MGC_END0_STAGE_STALL_BIT;

        stage=stage&~stage&MGC_END0_STAGE_STALL_BIT;

        sprintf(buff, "%s%s", (stallbit)? "stall-" : "",
                (stage==MGC_END0_STAGE_SETUP)
                ? "setup" :
                (stage==MGC_END0_STAGE_TX)
                ? "tx" :
                (stage==MGC_END0_STAGE_RX)
                ? "rx" :
                (stage==MGC_END0_STAGE_STATUSIN)
                ? "statusin" :
                (stage==MGC_END0_STAGE_STATUSOUT)
                ? "statusout" : "error");

        return buff;
}

void dump_all_regs(uint8_t* pBase, int multipoint, uint8_t bEnd)
{
        int i;
        struct {
                unsigned int offset;
                char *desc;
        }regs_list[] = {
                { 0x00, "FAddr Function address register." },
                { 0x01, "Power Power management register." },
                { 0x02, "IntrTx Interrupt register for Endpoint 0 plus Tx Endpoints 1 to 15." },
                { 0x03, "IntrTx Interrupt register for Endpoint 0 plus Tx Endpoints 1 to 15." },
                { 0x04, "IntrRx Interrupt register for Rx Endpoints 1 to 15." },
                { 0x05, "IntrRx Interrupt register for Rx Endpoints 1 to 15." },
                { 0x06, "IntrTxE Interrupt enable register for IntrTx." },
                { 0x07, "IntrTxE Interrupt enable register for IntrTx." },
                { 0x08, "IntrRxE Interrupt enable register for IntrRx." },
                { 0x09, "IntrRxE Interrupt enable register for IntrRx." },
                { 0x0A, "IntrUSB Interrupt register for common USB interrupts." },
                { 0x0B, "IntrUSBE Interrupt enable register for IntrUSB." },
                { 0x0C, "Frame Frame number." },
                { 0x0D, "Frame Frame number." },
                { 0x0E, "Index Index register for selecting the endpoint status and control registers." },
                { 0x10, "#TxMaxP Maximum packet size for host Tx endpoint. (Index register set to select Endpoints 1 to 5 only)" },
                { 0x11, "#TxMaxP Maximum packet size for host Tx endpoint. (Index register set to select Endpoints 1 to 5 only)" },
                { 0x12, "[TxCSR Control Status register for host Tx endpoint. (Index register set to select Endpoints 1 to 5)" },
                { 0x13, "[CSR0 Control Status register for 12,13 Endpoint 0. (Index register set to select Endpoint 0)" },
                { 0x14, "" },
                { 0x15, "^RxMaxP Maximum packet size for host Rx endpoint. (Index register set to select Endpoints 1 to 15 only)" },
                { 0x16, "" },
                { 0x17, "^RxCSR Control Status register for host Rx endpoint. (Index register set to select Endpoints 1 to 5 only)" },
                { 0x18, "[RxCount Number of bytes in host Rx endpoint FIFO. (Index register set to select Endpoints 1 to 5)" },
                { 0x19, "[Count0 Number of received bytes in Endpoint 0 FIFO. (Index register set to select Endpoint 0)" },
                { 0x1A, "[Type0 Defines the speed of Endpoint 0. (Index register set to select Endpoint 0)" },
                { 0x1A,	"[TxType Sets the transaction protocol, speed and peripheral endpoint number for the host Tx endpoint." },
                { 0x1B, "#NAKLimit0 Sets the NAK response timeout on Endpoint 0. (Index register set to select Endpoint 0)" },
                { 0x1B,	"#TxInterval Sets the polling interval for Interrupt/ISOC transactions or the NAK response timeout on Bulk transactions for host Tx endpoint." },
                { 0x1C, "RxType Sets the transaction protocol, speed and peripheral endpoint number for the host Rx endpoint." },
                { 0x1D, "RxInterval Sets the polling interval for Interrupt/ISOC transactions or the NAK response timeout on Bulk transactions for host Rx endpoint." },
                { 0x1F, "ConfigData Returns details of core configuration. (Index register set to select Endpoint 0.)" },
                { 0x60, "DevCtl OTG device control register." },
                { 0x62, "FIFOsz Tx Endpoint FIFO size" },
                { 0x63, "RxFIFOsz Rx Endpoint FIFO size" },
                { 0x64, "" },
                { 0x65, "^TxFIFOadd Tx Endpoint FIFO address" },
                { 0x66, "" },
                { 0x67, "^RxFIFOadd Rx Endpoint FIFO address" },
                { 0x78, "EPINFO Information about numbers of Tx and Rx endpoints." }
        };

        struct {
                int offset;
                char *desc;
        }regs_list2[] = {
                { 0x80, "TxFuncAddr Transmit Endpoint n Function Address (Host Mode only)" },
                { 0x82, "TxHubAddr Transmit Endpoint n Hub Address (Host Mode only) " },
                { 0x83, "TxHubPort Transmit Endpoint n Hub Port (Host Mode only) " },
                { 0x84, "RxFuncAddr Receive Endpoint n Function Address (Host Mode only) " },
                { 0x86, "RxHubAddr Receive Endpoint n Hub Address (Host Mode only) " },
                { 0x87, "RxHubPort Receive Endpoint n Hub Port (Host Mode only) " }
        };

        for (i = 0; i<sizeof(regs_list)/sizeof(regs_list[0]); i++)
                printk(" [%08x@%02x] = 0x%02x, %s \n",
                       (unsigned int)pBase, regs_list[i].offset,
                       MGC_Read8(pBase, regs_list[i].offset),
                       regs_list[i].desc);

        for (i = 0; i<sizeof(regs_list2)/sizeof(regs_list2[0]); i++)
                printk(" [%08x@%02x + 8*%d] = 0x%02x, %s \n",
                       (unsigned int)pBase, regs_list2[i].offset,
                       MGC_Read8(pBase, 0x0e),
                       MGC_Read8(pBase, regs_list2[i].offset+8*MGC_Read8(pBase, 0x0e)),
                       regs_list2[i].desc);

        return;
}
