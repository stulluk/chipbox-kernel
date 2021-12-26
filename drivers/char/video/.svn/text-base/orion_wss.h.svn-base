#ifndef __ORION_WSS_H__
#define __ORION_WSS_H__


#define TVE_OUTPUT_CONFIG			0x4

#define TVE_WSS_CONFIG			0x60
#define TVE_WSS_CLOCK_BASE 		0x61	
#define TVE_WSS_LEVEL_BASE		0x63	
#define TVE_WSS_LINEF0 			0x65			
#define TVE_WSS_LINEF1 			0x66
#define TVE_WSS_DATAF0_BASE 		0x67	
#define TVE_WSS_DATAF1_BASE 		0x6a		

#define TVE_BASE_ADDR 			0x10160000
#define ORION_WSS_BASE 			0x10160000
#define ORION_WSS_SIZE 			0x100


#define TVE_TTX_CONFIG				0x79
#define TVE_TTX_STARTPOS			0x7a
#define TVE_TTX_DATALENGTH	0x7b
#define TVE_TTX_FCODE				0x7c
#define TVE_TTX_CLOCK_BASE 		0x7d	
#define TVE_TTX_LEVEL_BASE			0x7f
#define TVE_TTX_LINEF0START_BASE 		0x81		
#define TVE_TTX_LINEF0END_BASE 			0x83		
#define TVE_TTX_LINEF1START_BASE 		0x85		
#define TVE_TTX_LINEF1END_BASE 			0x87		
#define TVE_TTX_LINEDISABLE_BASE 		0x89		



typedef enum VBI_WssStandard_ {
	VBI_WSS_PAL = 0,
	VBI_WSS_NTSC,
	VBI_WSS_END
} VBI_WssStandard_t;


typedef enum TVE_VsyncType_ {
	TVE_FIELD_TOP = 0,
	TVE_FIELD_BOT
} TVE_VsyncType_t;

typedef enum TVE_WssType_ {
	TVE_VBI_WSS = 0,
	TVE_VBI_CGMS,	
	TVE_VBI_END
} TVE_WssType_t;

typedef enum WSS_AspectRatio_ {
	WSS_AR_4TO3_FULL = 0,
	WSS_AR_14TO9_BOX_CENTER,	
	WSS_AR_14TO9_BOX_TOP,
	WSS_AR_16TO9_BOX_CENTER,
	WSS_AR_16TO9_BOX_TOP,
	WSS_AR_16TO9P_BOX_CENTER,
	WSS_AR_14TO9_FULL,	
	WSS_AR_16TO9_FULL,
	WSS_AR_END
} WSS_AspectRatio_t;



typedef enum  {
	VBI_TXT_PALB = 0,
	VBI_TXT_NTSCB,
	VBI_TXT_END
} VBI_TxtStandard_t;

typedef enum
{
	CSVOUT_COMP_YVU = 0,
	CSVOUT_COMP_RGB	

}CSVOUT_CompChannType_t;

#define TTX_LINESIZE          45
#define TTX_MAXLINES          32	

typedef struct
{
	unsigned int OddEvenFlag;
	
    unsigned int ValidLines;                     /* bit-field lines  0..31 */
    unsigned char  Lines[TTX_MAXLINES][TTX_LINESIZE];
} TTX_Page_t;



#if 0
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned int   U32;
#endif

typedef volatile u8  CSOS_DU8;


typedef struct VBI_Data_ {
	u8 size;
	u8 *Buf;
} VBI_Data_t;

typedef struct WSS_VbiData_ {
	TVE_WssType_t WssType;
	TVE_VsyncType_t FieldType;
	VBI_Data_t VbiData;
} WSS_VbiData_t;

typedef struct VBI_WssInfo_ {
	TVE_WssType_t WssType;
	WSS_AspectRatio_t ARatio;
} VBI_WssInfo_t;

/* wss Local */
#define wss_writeb(a,v)    	writeb(v, (ORION_WSS_BASE + (a)))
#define wss_writew(a,v)    	writew(v, (ORION_WSS_BASE + (a)))
#define wss_writel(a,v)    	writel(v, (ORION_WSS_BASE + (a)))

#define wss_readb(a)      	readb((ORION_WSS_BASE + (a)))
#define wss_readw(a)       	readw((ORION_WSS_BASE + (a)))
#define wss_readl(a)       	readl((ORION_WSS_BASE + (a)))

#define CSOS_WriteRegDev8(Address_p, Value)	do { 			\
	*((CSOS_DU8 *) (Address_p)) = (u8) (Value);  			\
} while (0)

//#define 	TVE_WriteRegDev8(base_addr, offset, data)		CSOS_WriteRegDev8((base_addr + offset), data)
#define TVE_WriteRegDev8(base_addr, offset, data)			\
	writeb(data, (base_addr + offset))

#define TVE_WriteRegDev32(base_addr, offset, data)			\
	writel(data, (base_addr + offset))

#define TVE_WriteRegDev16(base_addr, offset, data)			\
	writew(data, (base_addr + offset))


#define TVE_WriteRegDev10(base_addr, offset, value)	do {		\
	TVE_WriteRegDev8(base_addr, offset, ((value&0x300)>>8)); 	\
	TVE_WriteRegDev8(base_addr, (offset+1), (value&0xff));		\
} while (0)

#define TVE_WriteRegDev12(base_addr, offset, value) 	do { 		\
	TVE_WriteRegDev8(base_addr, offset, ((value&0xf00)>>8));	\
	TVE_WriteRegDev8(base_addr, (offset+1), (value&0xff));		\
} while (0)


#define TVE_WriteRegDev16LE(base_addr, offset, value) 	do { 		\
	TVE_WriteRegDev8(base_addr, offset, (value&0xff));		\
	TVE_WriteRegDev8(base_addr, (offset+1), ((value&0xff00)>>8));	\
} while (0)

#define TVE_WriteRegDev16BE(base_addr, offset, value) 	do { 		\
	TVE_WriteRegDev8(base_addr, offset, ((value&0xff00)>>8));	\
	TVE_WriteRegDev8(base_addr, (offset+1), (value&0xff));		\
} while (0)

#define TVE_WriteRegDev20BE(base_addr, offset, value) 	do { 		\
	TVE_WriteRegDev8(base_addr, offset, ((value&0xf0000)>>16));	\
	TVE_WriteRegDev8(base_addr, (offset+1), ((value&0xff00)>>8));	\
	TVE_WriteRegDev8(base_addr, (offset+2), (value&0xff));		\
} while (0)


#endif /* __ORION_WSS_H__ */

