
/*
 * PalmChip 3710 IDE Controller Driver Definitions
 */

#ifndef __PALM_BK3710_H__
#define __PALM_BK3710_H__

#include <asm/arch/hardware.h>

/*
 * PalmChip 3710 IDE Controller config Definitions
 */

#define IDE_PALM_REG_MMAP_BASE		0x10210000
#define IDE_PALM_IRQ			23
#define IDE_PALM_ATA_PRI_REG_OFFSET 	0x1F0	/* Offset of the primary interface registers */
#define IDE_PALM_ATA_PRI_CTL_OFFSET 	0x3F6	/* Primary Control Offset */

/*
 * PalmChip 3710 IDE Controller PIO cycle timing structure Definition
 */
typedef struct {
	unsigned int activetime;	/* Active Time 	*/
	unsigned int cycletime;		/* Cycle Time  	*/
}palm_bk3710_piotiming;

/*
 * PalmChip 3710 IDE Controller DMA cycle timing structure Definition
 */
typedef struct {
	unsigned int    activetime;	/* Active Time     */
	unsigned int    recoverytime;	/* Recovery Time   */
	unsigned int    cycletime;	/* Cycle Time      */
}palm_bk3710_dmatiming;

/*
 * PalmChip 3710 IDE Controller UDMA timing structure Definition
 */
typedef struct {
	unsigned int    envtime; 	/* Envelope Time        */
	unsigned int    rptime;		/* Ready to pause time  */
	unsigned int    cycletime;	/* Cycle Time           */
}palm_bk3710_udmatiming;

/* 
 * Register Overlay Structure for DmaEngine
 */
typedef struct {
	volatile unsigned short bmpcp;
	volatile unsigned short bmisp;
	volatile unsigned int 	bmidtp;
} palm_bk3710_dmaengineregs;

/*
 * Register Structure for Config
 */
typedef struct {
	volatile unsigned short	idetimp;
	volatile unsigned short	idetims;
	volatile unsigned char	sidetim;
	volatile unsigned char 	rsvd2[2];
	volatile unsigned char 	idestatus;
	volatile unsigned short udmactl;
	volatile unsigned short udmatim;
	volatile unsigned char 	rsvd0[4];
	volatile unsigned short miscctl;
	volatile unsigned char 	rsvd1[0x26];
	volatile unsigned int 	iordytmp;
} palm_bk3710_ideconfigregs;

/*
 * Register Structure
 */
typedef struct {
	palm_bk3710_dmaengineregs dmaengine;
	volatile unsigned char rsvd0[0x38];
	palm_bk3710_ideconfigregs config;
} palm_bk3710_ideregs;

#endif /* end of __PALM_BK3710_H__ */
