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
 * DMA Controller Abstraction (DCA) for the Inventra Controller Driver (ICD).
 * The ICD is a Linux USB Host Controller Driver (HCD) and optionally,
 * a Gadget provider.
 * $Revision: 1.1.1.1 $
 */

#ifndef __MGC_DMA_H__
#define __MGC_DMA_H__

/**
 * Introduction.
 * The purpose of the DMA Controller Abstraction (DCA) is to allow the ICD
 * to use any DMA controller,
 * since this is an option in the Inventra USB cores.
 * The assumptions are:
 * <ul>
 * <li>A DMA controller will be tied to an Inventra USB core in the
 * way specified in the Inventra core product specification.
 * <li>A DMA controller's base address in the memory map correlates
 * somehow to the Inventra USB core it serves.
 * </ul>
 * The responsibilities of an implementation include:
 * <ul>
 * <li>Allocating/releasing buffers for use with DMA
 * (this may be specific to a DMA controller, intervening busses,
 * and a target's capabilities,
 * so the ICD cannot make assumptions or provide services here)
 * <li>Handling the details of moving multiple USB packets
 * in cooperation with the Inventra USB core.
 * <li>Knowing the correlation between channels and the
 * Inventra core's local endpoint resources and data direction,
 * and maintaining a list of allocated/available channels.
 * <li>Updating channel status on interrupts,
 * whether shared with the Inventra core or separate.
 * <li>If the DMA interrupt is shared with the Inventra core,
 * handling it when called, and reporting whether it was the
 * source of interrupt.
 * </ul>
 */

/*************************** CONSTANTS ****************************/

/**
 * DMA channel status.
 */
typedef enum {
        /** A channel's status is unknown */
        MGC_DMA_STATUS_UNKNOWN,
        /** A channel is available (not busy and no errors) */
        MGC_DMA_STATUS_FREE,
        /** A channel is busy (not finished attempting its transactions) */
        MGC_DMA_STATUS_BUSY,
        /** A channel aborted its transactions due to a local bus error */
        MGC_DMA_STATUS_BUS_ABORT,
        /** A channel aborted its transactions due to a core error */
        MGC_DMA_STATUS_CORE_ABORT
} MGC_DmaChannelStatus;

/***************************** TYPES ******************************/

/**
 * MGC_DmaChannel.
 * A DMA channel.
 * @field pPrivateData channel-private data; not to be interpreted by the ICD
 * @field wMaxLength the maximum number of bytes the channel can move
 * in one transaction (typically representing many USB maximum-sized packets)
 * @field dwActualLength how many bytes have been transferred
 * @field bStatus current channel status (updated e.g. on interrupt)
 * @field bDesiredMode 0 if mode 1 is desired; -1 if mode 0 is desired
 */
typedef struct {
        void* pPrivateData;
        uint32_t dwMaxLength;
        uint32_t dwActualLength;
        MGC_DmaChannelStatus bStatus;
        uint8_t bDesiredMode;
} MGC_DmaChannel;

/**
 * Start a DMA controller.
 * @param pPrivateData private data pointer from MGC_DmaController
 * @return 0 on success
 * @return -1 on failure (e.g. no DMAC appears present)
 */
typedef uint8_t (*MGC_pfDmaStartController)(void* pPrivateData);

/**
 * Stop a DMA controller.
 * @param pPrivateData the controller's private data pointer
 * @return 0 on success
 * @return -1 on failure; the ICD may try again
 */
typedef uint8_t (*MGC_pfDmaStopController)(void* pPrivateData);

/**
 * Allocate a DMA channel.
 * Allocate a DMA channel suitable for the given conditions.
 * @param pPrivateData the controller's private data pointer
 * @param bLocalEnd the local endpoint index (1-15)
 * @param bTransmit 0 for transmit; -1 for receive
 * @param bProtocol the USB protocol, as per USB 2.0 chapter 9
 * (0 => control, 1 => isochronous, 2 => bulk, 3 => interrupt)
 * @param wMaxPacketSize maximum packet size
 * @return a non-NULL pointer on success
 * @return NULL on failure (no channel available)
 */
typedef MGC_DmaChannel* (*MGC_pfDmaAllocateChannel)(
        void* pPrivateData, uint8_t bLocalEnd,
        uint8_t bTransmit, uint8_t bProtocol, uint16_t wMaxPacketSize);

/**
 * Release a DMA channel.
 * Release a previously-allocated DMA channel.
 * The ICD guarantess to no longer reference this channel.
 * @param pChannel pointer to a channel obtained by
 * a successful call to pController->pfDmaAllocateChannel
 */
typedef void (*MGC_pfDmaReleaseChannel)(MGC_DmaChannel* pChannel);

/**
 * Allocate DMA buffer.
 * Allocate a buffer suitable for DMA operations with the given channel.
 * @param pChannel pointer to a channel obtained by
 * a successful call to pController->pfDmaAllocateChannel
 * @param dwLength length, in bytes, desired for the buffer
 * @return a non-NULL pointer to a suitable region (in processor space)
 * on success
 * @return NULL on failure
 */
typedef uint8_t* (*MGC_pfDmaAllocateBuffer)(MGC_DmaChannel* pChannel,
                uint32_t dwLength);

/**
 * Release DMA buffer.
 * Release a DMA buffer previously acquiring by a successful call
 * to pController->pfDmaAllocateBuffer.
 * @param pChannel pointer to a channel obtained by
 * a successful call to pController->pfDmaAllocateChannel
 * @param pBuffer the buffer pointer
 * @return 0 on success
 * @return -1 on failure (e.g. the controller owns the buffer at present)
 */
typedef uint8_t (*MGC_pfDmaReleaseBuffer)(MGC_DmaChannel* pChannel,
                uint8_t* pBuffer);

/**
 * Program a DMA channel.
 * Program a DMA channel to move data at the core's request.
 * The local core endpoint and direction should already be known,
 * since they are specified in the pfDmaAllocateChannel call.
 * @param pChannel pointer to a channel obtained by
 * a successful call to pController->pfDmaAllocateChannel
 * @param wPacketSize the packet size
 * @param bMode 0 if mode 1; -1 if mode 0
 * @param pBuffer base address of data (in processor space)
 * @param dwLength the number of bytes to transfer;
 * guaranteed by the ICD to be no larger than the channel's reported dwMaxLength
 * @return 0 on success
 * @return -1 on error
 */
typedef uint8_t (*MGC_pfDmaProgramChannel)(MGC_DmaChannel* pChannel,
                uint16_t wPacketSize, uint8_t bMode,
                const uint8_t* pBuffer,
                uint32_t dwLength,
               void * pThis
                );

/**
 * Get DMA channel status.
 * Get the current status of a DMA channel, if the hardware allows.
 * @param pChannel pointer to a channel obtained by
 * a successful call to pController->DmaAllocateChannel
 * @return current status
 * (MGC_DMA_STATUS_UNKNOWN if hardware does not have readable status)
 */
typedef MGC_DmaChannelStatus (*MGC_pfDmaGetChannelStatus)(
        MGC_DmaChannel* pChannel);

/**
 * DMA ISR.
 * If present, this function is called by the ICD on every interrupt.
 * This is necessary because with the built-in DMA controller
 * (and probably some other configurations),
 * the DMA interrupt is shared with other core interrupts.
 * Therefore, this function should return quickly
 * when there is no DMA interrupt.
 * When there is a DMA interrupt, this function should
 * perform any implementations-specific operations,
 * and update the status of all appropriate channels.
 * If the DMA controller has its own dedicated interrupt,
 * this function should do nothing.
 * This function is called BEFORE the ICD handles other interrupts.
 * @param pPrivateData the controller's private data pointer
 * @return 0 if an interrupt was serviced
 * @return -1 if no interrupt required servicing
 */
typedef uint8_t (*MGC_pfDmaControllerIsr)(void* pPrivateData);

/**
 * MGC_DmaController.
 * A DMA Controller.
 * This is in a struct to allow the ICD to support
 * multiple cores of different types,
 * since each may use a different type of DMA controller.
 * @field pPrivateData controller-private data;
 * not to be interpreted by the UCD
 * @field pfDmaStartController ICD calls this to start a DMA controller
 * @field pfDmaStopController ICD calls this to stop a DMA controller
 * @field pfDmaAllocateChannel ICD calls this to allocate a DMA channel
 * @field pfDmaReleaseChannel ICD calls this to release a DMA channel
 * @field pfDmaAllocateBuffer ICD calls this to allocate a DMA buffer
 * @field pfDmaReleaseBuffer ICD calls this to release a DMA buffer
 * @field pfDmaGetChannelStatus ICD calls this to get a DMA channel's status
 * @field pfDmaControllerIsr ICD calls this (if non-NULL) from its ISR
 */
typedef struct {
        void* pPrivateData;
        MGC_pfDmaStartController pfDmaStartController;
        MGC_pfDmaStopController pfDmaStopController;
        MGC_pfDmaAllocateChannel pfDmaAllocateChannel;
        MGC_pfDmaReleaseChannel pfDmaReleaseChannel;
        MGC_pfDmaAllocateBuffer pfDmaAllocateBuffer;
        MGC_pfDmaReleaseBuffer pfDmaReleaseBuffer;
        MGC_pfDmaProgramChannel pfDmaProgramChannel;
        MGC_pfDmaGetChannelStatus pfDmaGetChannelStatus;
        MGC_pfDmaControllerIsr pfDmaControllerIsr;
} MGC_DmaController;

/**
 * A DMA channel has new status.
 * This may be used to notify the ICD of channel status changes asynchronously.
 * This is useful if the DMA interrupt is different from the USB controller's
 * interrupt, so on some systems there may be no control over the order of
 * USB controller and DMA controller assertion.
 * @param pPrivateData the controller's private data pointer
 * @param bLocalEnd the local endpoint index (1-15)
 * @param bTransmit 0 for transmit; -1 for receive
 * @return 0 if an IRP was completed as a result of this call;
 * -1 otherwise
 */
typedef uint8_t (*MGC_pfDmaChannelStatusChanged)(
        void* pPrivateData, uint8_t bLocalEnd,
        uint8_t bTransmit);

/**
 * Instantiate a DMA controller.
 * Instantiate a software object representing a DMA controller.
 * @param pfDmaChannelStatusChanged channel status change notification function.
 * Normally, the ICD requests status in its interrupt handler.
 * For some DMA controllers, this may not be the correct time.
 * @param pDmaPrivate parameter for pfDmaChannelStatusChanged
 * @param pCoreBase the base address (in kernel space) of the core
 * It is assumed the DMA controller's registers' base address will be related
 * to this in some way.
 * @return non-NULL pointer on success
 * @return NULL on failure (out of memory or exhausted
 * a fixed number of controllers)
 */
typedef MGC_DmaController* (*MGC_pfNewDmaController)(
        MGC_pfDmaChannelStatusChanged pfDmaChannelStatusChanged,
        void* pDmaPrivate,
        uint8_t* pCoreBase);

/**
 * Destroy DMA controller.
 * Destroy a previously-instantiated DMA controller.
 */
typedef void (*MGC_pfDestroyDmaController)(
        MGC_DmaController* pController);

/**
 * MGC_DmaControllerFactory.
 * A DMA controller factory.
 * To allow for multi-core implementations and different
 * types of cores and DMA controllers to co-exist,
 * it is necessary to create them from factories.
 * @field wCoreRegistersExtent the total size of the core's
 * register region with the DMA controller present,
 * for use in mapping the core into system memory.
 * For example, the MHDRC core takes 0x200 bytes of address space.
 * If your DMA controller starts at 0x200 and takes 0x100 bytes,
 * set this to 0x300.
 * @field pfNewDmaController create a DMA controller
 * @field pfDestroyDmaController destroy a DMA controller
 */
typedef struct {
        uint16_t wCoreRegistersExtent;
        MGC_pfNewDmaController pfNewDmaController;
        MGC_pfDestroyDmaController pfDestroyDmaController;
} MGC_DmaControllerFactory;

#endif	/* multiple inclusion protection */
