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

/* USB High-Speed Dual-Role Controller Configuration
*/
/* Copyright Mentor Graphics Corporation and Licensors 2003
*/
/* V1.000
*/

/* musbhdrc_cfg
*/
/* This file contains configuration constants for the musbhdrc.
*/

/* ** Number of Tx endpoints ***/
/* Legal values are 1 - 16 (this value includes EP0)*/
/* ide, mouse, keyboard, */
#define MGC_C_NUM_EPT 4

/* ** Number of Rx endpoints ***/
/* Legal values are 1 - 16 (this value includes EP0)*/
#define MGC_C_NUM_EPR 4

/* ** Endpoint 1 to 15 direction types ***/
/* C_EP1_DEF is defined if either Tx endpoint 1 or Rx endpoint 1 are used*/
#define MGC_C_EP1_DEF

/* C_EP1_TX_DEF is defined if Tx endpoint 1 is used*/
#define MGC_C_EP1_TX_DEF

/* C_EP1_RX_DEF is defined if Rx endpoint 1 is used*/
#define MGC_C_EP1_RX_DEF

/* C_EP1_TOR_DEF is defined if Tx endpoint 1 and Rx endpoint 1 share a FIFO*/
/*`define C_EP1_TOR_DEF*/

/* C_EP1_TAR_DEF is defined if both Tx endpoint 1 and Rx endpoint 1 are used*/
/* and do not share a FIFO*/
#define MGC_C_EP1_TAR_DEF

/* Similarly for all other used endpoints*/
#define MGC_C_EP2_DEF
#define MGC_C_EP2_TX_DEF
#define MGC_C_EP2_RX_DEF
#define MGC_C_EP2_TAR_DEF

#define MGC_C_EP3_DEF
#define MGC_C_EP3_TX_DEF
#define MGC_C_EP3_RX_DEF
#define MGC_C_EP3_TAR_DEF

/* `define C_EP4_DEF*/
/* `define C_EP4_TX_DEF*/
/* `define C_EP4_RX_DEF*/
/* `define C_EP4_TAR_DEF*/

/* `define C_EP5_DEF*/
/* `define C_EP5_TX_DEF*/
/* `define C_EP5_RX_DEF*/
/* `define C_EP5_TAR_DEF*/

/* ** Endpoint 1 to 15 FIFO address bits ***/
/* Legal values are 3 to 13 - corresponding to FIFO sizes of 8 to 8192 bytes.*/
/* If an Tx endpoint shares a FIFO with an Rx endpoint then the Rx FIFO size*/
/* must be the same as the Tx FIFO size.*/
/* All endpoints 1 to 15 must be defined, unused endpoints should be set to 2.*/
#define MGC_C_EP1T_BITS 10
#define MGC_C_EP1R_BITS 10
#define MGC_C_EP2T_BITS 10
#define MGC_C_EP2R_BITS 10
#define MGC_C_EP3T_BITS 10
#define MGC_C_EP3R_BITS 10
#define MGC_C_EP4T_BITS 2
#define MGC_C_EP4R_BITS 2
#define MGC_C_EP5T_BITS 2
#define MGC_C_EP5R_BITS 2
#define MGC_C_EP6T_BITS 2
#define MGC_C_EP6R_BITS 2
#define MGC_C_EP7T_BITS 2
#define MGC_C_EP7R_BITS 2
#define MGC_C_EP8T_BITS 2
#define MGC_C_EP8R_BITS 2
#define MGC_C_EP9T_BITS 2
#define MGC_C_EP9R_BITS 2
#define MGC_C_EP10T_BITS 2
#define MGC_C_EP10R_BITS 2
#define MGC_C_EP11T_BITS 2
#define MGC_C_EP11R_BITS 2
#define MGC_C_EP12T_BITS 2
#define MGC_C_EP12R_BITS 2
#define MGC_C_EP13T_BITS 2
#define MGC_C_EP13R_BITS 2
#define MGC_C_EP14T_BITS 2
#define MGC_C_EP14R_BITS 2
#define MGC_C_EP15T_BITS 2
#define MGC_C_EP15R_BITS 2

/* Define the following constant if the USB2.0 Transceiver Macrocell data width is 16-bits.*/
/* `define C_UTM_16*/

/* Define this constant if the CPU uses big-endian byte ordering.*/
/*`define C_BIGEND*/

/* Define the following constant if any Tx endpoint is required to support multiple bulk packets.*/
/* `define C_MP_TX*/

/* Define the following constant if any Rx endpoint is required to support multiple bulk packets.*/
/* `define C_MP_RX*/

/* Define the following constant if any Tx endpoint is required to support high bandwidth ISO.*/
/* `define C_HB_TX*/

/* Define the following constant if any Rx endpoint is required to support high bandwidth ISO.*/
/* `define C_HB_RX*/

/* Define the following constant if late DMA deassertion timing is required for Tx endpoints.*/
/* `define C_LDMA_TX*/

/* Define the following constant if late DMA deassertion timing is required for Rx endpoints.*/
/* `define C_LDMA_RX*/

/* Define the following constants for ULPI PHY interface width.*/
/* `define C_DDR*/
/* `define C_DDR_HW*/
#define MGC_C_LPI_WIDTH 7

/* added by yan@10-25-2006 11:45 */
#define MGC_C_LPI

/* Define the following constant if software connect/disconnect control is required.*/
#define C_SOFT_CON

/* Define the following constant if Vendor Control Registers are required.*/
/* `define C_VEND_REG*/

/* Vendor control register widths.*/
#define MGC_C_VCTL_BITS 4
#define MGC_C_VSTAT_BITS 8


/* Define the following constant to include a DMA controller.*/
#define MGC_C_DMA

/* Define the following constant if 2 or more DMA channels are required.*/
#define MGC_C_DMA2

/* Define the following constant if 3 or more DMA channels are required.*/
/*`define C_DMA3*/

/* Define the following constant if 4 or more DMA channels are required.*/
/*`define C_DMA4*/

/* Define the following constant if 5 or more DMA channels are required.*/
/*`define C_DMA5*/

/* Define the following constant if 6 or more DMA channels are required.*/
/*`define C_DMA6*/

/* Define the following constant if 7 or more DMA channels are required.*/
/*`define C_DMA7*/

/* Define the following constant if 8 or more DMA channels are required.*/
/*`define C_DMA8*/

/* ** Derived constants ***/
/* The following constants are derived from the previous configuration constants*/

/* Enable Dynamic FIFO Sizing, */
#define MGC_C_DYNFIFO_DEF

/* Total number of endpoints*/
/* Legal values are 2 - 16*/
/* This must be equal to the larger of C_NUM_EPT, C_NUM_EPR*/
#define MGC_C_NUM_EPS 4

/* C_EPMAX_BITS is equal to the largest endpoint FIFO word address bits*/
#define MGC_C_EPMAX_BITS 9

/* C_RAM_BITS is the number of address bits required to address the RAM (32-bit addresses)*/
/* It is defined as log2 of the sum of 2** of all the endpoint FIFO dword address bits (rounded up).*/
/* 2 x 1024 to support any possible ep(max bulk 512'B, max iso = 1024'B)*/
/*`define C_RAM_BITS 11*/
#define MGC_C_RAM_BITS 9


