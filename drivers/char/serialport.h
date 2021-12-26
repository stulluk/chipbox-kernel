/*
 * Linux Serial Port Header File
 *
 * UART0 is used for communication with PC for downloading U-boot, kernel and application via x/y/z - modem protocols
 * UART1 will be used to communicate with Frontpanel for IR
 */

#ifndef __CS_SERIAL_RC_H__
#define __CS_SERIAL_RC_H__


/*Globals for Driver here */

/*Define Register Maps here */
#define 	MASK		0x000F			/*only (LSB) 8 bits of 32 is used in the registers below */
#define 	MASK_MCR	0x001F			/*mask for Modem Control Register */

#define		UART1_BASE 	0x101F2000		/*Orion Base for UART */
#define		UART1_BASE_SIZE 0x100			/*Total Size of Register starting from Base Required */
#define		UART1_RBR 	(0x00 >> 1)		/*Receiver Buffer Register */
#define		UART1_THR 	(0x00 >> 1)		/*Transmit Holding Register */
#define		UART1_DLL 	(0x00 >> 1)		/*Divisor Latch Low */
#define		UART1_DLH 	(0x04 >> 1)		/*Divisor Latch High */
#define		UART1_IER 	(0x04 >> 1)		/*Interrupt Enable Register */
#define		UART1_IIR 	(0x08 >> 1)		/*Interrup Identity Register */
#define		UART1_FCR 	(0x08 >> 1)		/*FIFO Control Register */
#define		UART1_LCR 	(0x0C >> 1)		/*Line Control Register */
#define		UART1_MCR 	(0x10 >> 1)		/*Modem Control Register */
#define		UART1_LSR 	(0x14 >> 1)		/*Line Status Register */
#define		UART1_MSR 	(0x18 >> 1)		/*Modem Status Register */

//#define		UART1_SCR 	(0x1C >> 1)
//#define		UART1_LPDLL 	(0x20 >> 1)
//#define		UART1_LPDLH 	(0x24 >> 1)
//#define		UART1_SRBR 	(0x30 >> 1)
//#define		UART1_STHR 	(0x30 >> 1)

#define		UART1_FAR 	(0x70 >> 1)		/*FIFO Access Register */
#define		UART1_TFR 	(0x74 >> 1)		/*Transmit FIFO Read */
#define		UART1_RFW 	(0x78 >> 1)		/*Receive FIFO Write */
#define		UART1_USR 	(0x7C >> 1)		/*UART Status Register */

//#define		UART1_TFL 	(0x80 >> 1)
//#define		UART1_RFL 	(0x84 >> 1)
//#define		UART1_SRR 	(0x88 >> 1)
//#define		UART1_SRTS 	(0x8C >> 1)
//#define		UART1_SBCR 	(0x90 >> 1)
//#define		UART1_SDMAM 	(0x94 >> 1)
//#define		UART1_SFE 	(0x98 >> 1)
//#define		UART1_SRT 	(0x9C >> 1)
//#define		UART1_STET 	(0xA0 >> 1)

#define		UART1_HTX 	(0xA4)			/*Halt Transmission */

//#define		UART1_DMAS 	(0xA8 >> 1)
//#define		UART1_CPR 	(0xF4 >> 1)
//#define		UART1_UCV 	(0xF8 >> 1)
//#define		UART1_CTR 	(0xFC >> 1)


/*Define IOCTLS for Serial Port Settings */

int serialport_major = 0;				/*Dynamic Allocation of Major number */
int UART1_IRQ = 13;					/* IRQ of Serial Port */

#define		SP_MAGIC			'j'
#define 	SP_SET_UART_CONFIG		_IOW(SP_MAGIC, 1, int)
#define 	SP_SET_UART_LOOPBACK		_IOW(SP_MAGIC, 2, int)
#define 	SP_SET_RST_LOOPBACK		_IOW(SP_MAGIC, 3, int)
#define 	SP_SET_IR_MODE			_IOW(SP_MAGIC, 4, int)
#define 	SP_SET_IR_LOOPBACK		_IOW(SP_MAGIC, 5, int)
#define 	SP_WR_TX_DATA_UART		_IOW(SP_MAGIC, 6, int)
#define 	SP_SET_ENABLE_SERIALIN		_IOW(SP_MAGIC, 7, int)
#define 	SP_SET_DISABLE_SERIALIN		_IOW(SP_MAGIC, 8, int)
#define 	SP_SET_RESET_SERIALIN		_IOW(SP_MAGIC, 9, int)
#define 	SP_SET_SERIALIN_ATTR		_IOW(SP_MAGIC, 10, int)
#define 	SP_SET_LCR			_IOW(SP_MAGIC, 11, int)
#define 	SP_GET_RBR_DATA			_IOW(SP_MAGIC, 12, int)

#endif /* __CS_SERIAL_RC_H__ */

/*End of File */
