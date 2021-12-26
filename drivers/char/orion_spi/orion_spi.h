#ifndef __ORION_SPI_H__
#define __ORION_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Debug Micros */
#undef ORION_DEBUG
#define ORION_DEBUG

#undef SPI_IRQ_POLICY
/* Micros */

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

#define SPI_TXEIS	0x1	/*Transmit fifo empty , transmit  fifo is equal to or below its threshold, auto clare*/
#define SPI_TXOIS	0x2	/*Transmit fifo overflow, set when apb access atempts to wo write into th e transmit*/
                                /*fifo atter it completely full ,auto clare */
#define SPI_RXUIS	0x4	/*Receive fifo  underflow,generatie if you attempt to read from an empty fifo,must read*/
                                /*RXUICR to clear it  */
#define SPI_RXOIS	0x8	/*Receive fifo overflow,receive logic atempty to place data into the receive fifo after */
				/*if completely filled ,must read the RXOICR to clare ti*/
#define SPI_RXFIS	0x10	/*Receive fifo full,receive fifo is equal to or above its threthold plus 1,no need to clare it*/
#define SPI_MSTIS	0x20	/*the IMR bit : 1:unmask,0:musked,reset status :1,unmust  */

#define SPI_SR_BUSY    	0X1	/* 0:SSI BUSY, 1:IDLE OR DISABLE*/
#define SPI_SR_TFNF 	0X2	/*TRANSMIT FIFO NO  FULL ,(1 OR MORE LOCAL PLACE),0:FULL; 1:NOT FULL*/
#define SPI_SR_TFE     	0x4 	/*transmit fifo complite empty .0:not empty ; 1:empty */
#define SPI_SR_RFNE    	0x8	/*Receive fifo  not empty. 0:empty;1 not  empty*/
#define SPI_SR_RFF     	0x10	/*Receive fifo full */
#define SPI_SR_DCOL	0x40	/*multi-muster connection error   */

/* Ioctl */
#define SPI_MAGIC 'S'

#define SPI_BUS_STAT   		_IOW(SPI_MAGIC, 0, unsigned long)
#define SPI_SLAVE_STAT 		_IOW(SPI_MAGIC, 1, unsigned long)
#define SPI_SET_BAUD   		_IOW(SPI_MAGIC, 2, unsigned long)
#define SPI_SET_TMOD   		_IOW(SPI_MAGIC, 3, unsigned long)
#define SPI_SET_POL		_IOW(SPI_MAGIC, 4, unsigned long)
#define SPI_SET_PH		_IOW(SPI_MAGIC, 5, unsigned long)
#define SPI_SET_TIMING		_IOW(SPI_MAGIC, 6, unsigned long)
#define SPI_SET_NDF		_IOW(SPI_MAGIC, 12, unsigned long)

#define SPI_FLASH_R  		_IOW(SPI_MAGIC, 7, unsigned long)
#define SPI_FLASH_W  		_IOW(SPI_MAGIC, 8, unsigned long)
#define SPI_FLASH_E  		_IOW(SPI_MAGIC, 9, unsigned long)

#define SPI_READ_BYTE		_IOW(SPI_MAGIC, 10, unsigned long)
#define SPI_WRITE_BYTE		_IOW(SPI_MAGIC, 11, unsigned long)

/* 
 * Bus state choise
 * @ enable
 * @ disable
 */
typedef enum CSSPI_SPI_{
	CSSPI_ENABLE = 0,
	CSSPI_DISABLE } CSSPI_SPI;

/*
 * Slave select
 * @ get cs
 * @ drop cs
 */
typedef enum CSSPI_SLAVE_{
	CSSPI_SENABLE = 0,
	CSSPI_SDISABLE } CSSPI_SLAVE;

typedef enum CSSPI_TIMING_ {
	CSSPI_MODE0 = 0,
	CSSPI_MODE1,
	CSSPI_MODE2,
	CSSPI_MODE3 } CSSPI_TIMING;

/*
 * Transfer mode
 * @ trans and receive
 * @ trans only
 * @ receive only
 * @ reservered
 */
typedef enum CSSPI_TMOD_ {
	CSSPI_TXRX = 0,
	CSSPI_TXO,
	CSSPI_RXO,
	CSSPI_EEPROM } CSSPI_TMOD;   

typedef enum CSSPI_SCPOL_ {
	CSSPI_LOWPOL= 0,
	CSSPI_HIGHPOL } CSSPI_SCPOL;

typedef enum CSSPI_SCPH_ {
	CSSPI_MIDDLE = 0,
	CSSPI_START } CSSPI_SCPH;

/*
 * Baud rate
 * @ 1MHz
 * @ 2MHz
 * @ 3MHz
 * @ 4MHz
 * @ 5MHz
 */
typedef enum CSSPI_SCBAUD_ {
	CSSPI_500K = 0,
	CSSPI_1MH,
	CSSPI_2MH,
	CSSPI_3MH,
	CSSPI_4MH,
	CSSPI_5MH } CSSPI_SCBAUD;

/*
 * loop queue definitions and implement 
 */
/* TODO: This value is ensure (rq.size) is not exceed our expection. */
#define IFD_BUF_SIZE (512)
typedef struct dat_queue {
	u32 size;
	u32 head;
	u32 tail;
	u8  buf[IFD_BUF_SIZE];
} dat_queue_t;

#define INC(a) 			((a) = ((a)+1) & (IFD_BUF_SIZE-1))
#define DEC(a) 			((a) = ((a)-1) & (IFD_BUF_SIZE-1))
#define EMPTY(a) 		((a).head == (a).tail)
#define CHARS(a) 		(((a).head-(a).tail-1)&(IFD_BUF_SIZE-1))
#define LEFT(a) 		(((a).tail-(a).head)&(IFD_BUF_SIZE-1))
#define LAST(a) 		((a).buf[(IFD_BUF_SIZE-1)&((a).head-1)])
#define FULL(a) 		((a).head == (((a).tail+1) & (IFD_BUF_SIZE-1)))
#define GETCH(queue,c) 	(void)({c=(queue).buf[(queue).head];INC((queue).head);})
#define PUTCH(c,queue) 	(void)({(queue).buf[(queue).tail]=(c);INC((queue).tail);})

typedef struct orion_spi {
	u8           	baud;
	u8		alen;
	u32		size;
	u8		open_cnt;

#ifdef CONFIG_SPI_COMMON
#define RXTIMEOUT_DEFAULT 10
	u8		rxtimeout;
	u8		receive_exception;
	struct semaphore	tq_sem;
	wait_queue_head_t	rq_queue;	
	wait_queue_head_t	tq_queue;	
	dat_queue_t	rq;
	dat_queue_t	tq;
#endif
	struct tasklet_struct	tasklet;

	spinlock_t	spi_lock;
} orion_spi_t;

extern u32 orion_spi_reg_read(void);
extern u32 orion_spi_reg_write(u8 *buf, int len);
extern void spi_mode0(void);
extern void spi_mode3(void);

extern u32 orion_spi_simple_read(u32 reg);
extern u32 orion_spi_simple_write(u32 reg, u32 val);

#ifdef __cplusplus
}
#endif

#endif /* __ORION_SPI_H__ */
