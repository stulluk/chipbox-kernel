#ifndef __ORION_SPI_DEVICE_H__
#define __ORION_SPI_DEVICE_H__

/* Register Offset  */
#define ORION_SPI_BASE 	0x10173000
#define ORION_SPI_SIZE 	0x100
#define ORION_SPI_IRQ 	11

#define BOOTCFG         0x10100034

/* Registers */
#define rCTRLR0	       	0x00
#define rCTRLR1	       	0x04
#define rSSIENR	       	0x08
#define rMWCR  	       	0x0C
#define rSER   	       	0x10
#define rBAUDR    	0x14
#define rTXFTLR 	0x18
#define rRXFTLR 	0x1C
#define rTXFLR  	0x20
#define rRXFLR  	0x24
#define rSR    		0x28
#define rIMR   		0x2C
#define rISR   		0x30
#define rRISR  		0x34
#define rTXOICR 	0x38
#define rRXOICR 	0x3C
#define rRXUICR		0x40
#define rMSTICR		0x44
#define rICR   		0x48
#define rDR    		0x60

/* Register operation */
#define spi_writeb(a,v)    	writeb(v, (flash->base + (a)))
#define spi_write(a,v)    	writew(v, (flash->base + (a)))
#define spi_writel(a,v)    	writel(v, (flash->base + (a)))

#define spi_readb(a)      	readb((flash->base + (a)))
#define spi_read(a)       	readw((flash->base + (a)))
#define spi_readl(a)       	readl((flash->base + (a)))

/********************************************************************/
#define DEF_R_SPEED 	0x5
#define DEF_W_SPEED 	0xa
/********************************************************************/

static inline void *kzalloc(size_t size, unsigned int flags)
{
	void *mem = kmalloc(size, flags);
	memset(mem, 0, size);
	return mem;
}

typedef enum {
	TMOD_TXRX = 0,
	TMOD_TX   = 1,
	TMOD_RX   = 2,
	TMOD_EE   = 3 } TMOD_E;
typedef enum {
	TIMING0   = 0,
	TIMING1   = 1,
	TIMING2   = 2,
	TIMING3   = 3   } TIMING_E;

#define ADDRESS_LEN 	3

#define CMD_RDSR 	0x05
#define CMD_WRSR 	0x01
#define CMD_READ 	0x03
#define CMD_WREN 	0x06
#define CMD_WRITE 	0x02
#define CMD_ER_SEC 	0x20
#define CMD_ER_BLK 	0xd8
#define CMD_ER_CHIP 	0xc7
#define CMD_RDID 	0x9f

#define MAX_RCV_SZ 	1024
#define RD_SZ 		MAX_RCV_SZ
#define PP_SZ 		256

#define PP_TIME_US 	1500

#endif
