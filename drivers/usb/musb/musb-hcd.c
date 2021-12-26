#include <linux/config.h>



#define MGC_DEBUG 	CONFIG_USB_INVENTRA_HCD_LOGGING
#define MGC_DMA
#include "virthub.c"
#include "musb_driver.c"
#include "musb_host.c"
#include "musb_buffer.c"
#include "debug.c"
//#include "musbhsdma.c"
