/*
 * DMA memory management for framework level HCD code (hc_driver)
 *
 * This implementation plugs in through generic "usb_bus" level methods,
 * and should work with all USB controllers, regardles of bus type.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <asm/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>

#define MGC_BUFFER_POOLS 4
#ifdef CONFIG_USB_DEBUG
	#define DEBUG
#else
	#undef DEBUG
#endif

#include <linux/usb.h>
//#include "hcd.h"


/*
 * DMA-Coherent Buffers
 */

/* FIXME tune these based on pool statistics ... */
static const size_t	pool_max [MGC_BUFFER_POOLS] = {
	/* platforms without dma-friendly caches might need to
	 * prevent cacheline sharing...
	 */
	32,
	128,
	512,
	PAGE_SIZE / 2
	/* bigger --> allocate pages */
};


/* SETUP primitives */

/**
 * hcd_buffer_create - initialize buffer pools
 * @hcd: the bus whose buffer pools are to be initialized
 * Context: !in_interrupt()
 *
 * Call this as part of initializing a host controller that uses the dma
 * memory allocators.  It initializes some pools of dma-coherent memory that
 * will be shared by all drivers using that controller, or returns a negative
 * errno value on error.
 *
 * Call hcd_buffer_destroy() to clean up after using those pools.
 */
int musb_buffer_create (MGC_LinuxCd* pThis)
{
	char		name [16];
	int 		i, size;

	for (i = 0; i < MGC_BUFFER_POOLS; i++) { 
		if (!(size = pool_max [i]))
			continue;
		snprintf (name, sizeof name, "buffer-%d", size);
		
		pThis->pool [i] = dma_pool_create (name, pThis->pBus->controller,
			size, size, 0);
		if (!pThis->pool [i]) {
			musb_buffer_destroy (pThis);
			return -ENOMEM;
		}
	}
	return 0;
}


/**
 * hcd_buffer_destroy - deallocate buffer pools
 * @hcd: the bus whose buffer pools are to be destroyed
 * Context: !in_interrupt()
 *
 * This frees the buffer pools created by hcd_buffer_create().
 */
void musb_buffer_destroy (MGC_LinuxCd* pThis)
{
	int		i;

	for (i = 0; i <MGC_BUFFER_POOLS; i++) { 
		struct dma_pool	*pool = pThis->pool [i];
		if (pool) {
			dma_pool_destroy (pool);
			pThis->pool[i] = NULL;
		}
	}
}


/* sometimes alloc/free could use kmalloc with SLAB_DMA, for
 * better sharing and to leverage mm/slab.c intelligence.
 */

void *musb_buffer_alloc (
	struct usb_bus 		*bus,
	size_t			size,
	int			mem_flags,
	dma_addr_t		*dma
)
{
	MGC_LinuxCd *pThis = bus->hcpriv;
	int 			i;

	for (i = 0; i < MGC_BUFFER_POOLS; i++) {
		if (size <= pool_max [i])
			return dma_pool_alloc (pThis->pool [i], mem_flags, dma);
	}
	return dma_alloc_coherent (pThis->pBus->controller, size, dma, 0);
	
}

void musb_buffer_free (
	struct usb_bus 		*bus,
	size_t			size,
	void 			*addr,
	dma_addr_t		dma
)
{
	MGC_LinuxCd *pThis = bus->hcpriv;
	int 			i;

	if (!addr)
		return;
#if 0
	if (!bus->controller->dma_mask) {
		kfree (addr);
		return;
	}
#endif
	for (i = 0; i <MGC_BUFFER_POOLS; i++) {
		if (size <= pool_max [i]) {
			dma_pool_free (pThis->pool [i], addr, dma);
			return;
		}
	}
	dma_free_coherent (pThis->pBus->controller, size, addr, dma);
}

