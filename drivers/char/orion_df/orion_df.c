/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2008 Celestial Semiconductor Inc.
// All rights reserved
// [RELEASE HISTORY]                           
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.1      08-10-10   Jia Ma           			Original
// ---------------------------------------------------------------------------
//*/
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/devfs_fs_kernel.h>
//#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/mem_define.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/proc_fs.h>
#include <linux/device.h>

#include <linux/dmapool.h>

#include "df_reg_def.h"
#include "df_reg_fmt.h"
#include "orion_df.h"
#include "scaler_coeff.h"

#include "tve0_1201.h"
#include "tve1_1201.h"
#include "orion_wss.h"

MODULE_AUTHOR("Jia Ma, <jia.ma@celestialsemi.com>");
MODULE_DESCRIPTION("Celestial Semiconductor Display feeder sub-system driver");
MODULE_LICENSE("GPL");
/************************************* GLOBAL VARIABLE *************************************/
#define DEFAULT_SCALER

static int __Is_ColorBar = 0;
static int __IsSysPll = 1;
static int __Is_first = 1;
int __VGA_Support = 0;
int __Is_TV = 1;

extern unsigned int g_audio_currnet_freq;
extern void video_write(unsigned int val, unsigned int addr);
extern unsigned int video_read(unsigned int addr);
volatile unsigned char  *disp_base = NULL;
volatile unsigned char  *tve0_base = NULL;
volatile unsigned char  *tve1_base = NULL;
volatile unsigned char  *clk_base = NULL;

volatile unsigned char  *wss_base = NULL;
volatile unsigned char  *ttx_ctrl = NULL;

volatile unsigned char  *ttx_buf = NULL;
struct dma_pool *pool;
dma_addr_t dma_phy_addr;

struct df_output_pos out_pos;
static struct class_simple *csm_df_class;

static dfdev df_dev[8];
static int tve_2_df[] = { 
	DISP_YUV_PAL,
	DISP_YUV_NTSC,
	DISP_YUV_576P,
	DISP_YUV_480P,
	DISP_YUV_720P_50FPS,
	DISP_YUV_720P_60FPS,
	DISP_YUV_1080I_25FPS,
	DISP_YUV_1080I_30FPS,

	DISP_YUV_PAL,
	DISP_YUV_NTSC,
	DISP_YUV_PAL,
	DISP_YUV_PAL,
	DISP_YUV_1080P_24FPS,
	DISP_YUV_1080P_25FPS,
	DISP_YUV_1080P_30FPS,

	DISP_RGB_640X480_60FPS,
	DISP_RGB_800X600_60FPS,
	DISP_RGB_800X600_72FPS,
	DISP_RGB_1024X768_60FPS,
	DISP_RGB_1280X1024_50FPS,
	DISP_RGB_1600X1000_60FPS,
       DISP_RGB_1280X1024_60FPS,
       DISP_RGB_1280x720_60FPS,
       DISP_RGB_848X480_60FPS,
       DISP_RGB_800X480_60FPS,
};

df_reg_para dfreg;

#if !defined(CONFIG_ARCH_ORION_CSM1201_J)
struct{
	int gfx_output[2];
	int vid_output[2];
	int IsTV;
	TVOUT_OUTPUT_MODE output_mode[2];
}layer_output = {
	.gfx_output = {0,0},
	.vid_output = {0,1},
	.IsTV = 1,
	.output_mode = {OUTPUT_MODE_YPBPR, OUTPUT_MODE_YPBPR},
};
#else
struct{
	int gfx_output[2];
	int vid_output[2];
	int IsTV;
	TVOUT_OUTPUT_MODE output_mode[2];
}layer_output = {
	.gfx_output = {1,1},
	.vid_output = {1,1},
	.IsTV = 1,
	.output_mode = {OUTPUT_MODE_YPBPR, OUTPUT_MODE_CVBS_SVIDEO},
};
#endif

void __pin_mux0_write(unsigned int value);
void __pin_mux1_write(unsigned int value);
unsigned int __pin_mux0_read(void);
unsigned int __pin_mux1_read(void);
extern int __csdrv_aud_set_freq(unsigned int audio_freq);
/**************************************** MACRO ***************************************/
#ifdef CONFIG_KERNEL_DF_DEBUG
#define DEBUG_PRINTF  printk
#else
#define DEBUG_PRINTF(fmt,args...)
#endif

#define DF_MAJOR			200
#define DF_MINOR_GFX0		0
#define DF_MINOR_GFX1		1
#define DF_MINOR_VID0		2	
#define DF_MINOR_VID1		3
#define DF_MINOR_OUT0		4
#define DF_MINOR_OUT1		5
#define DF_MINOR_TVE0		6
#define DF_MINOR_TVE1		7

#if 0
#define df_read(a)		*(disp_base + (a - DISP_REG_BASE))
#define df_write(a, d)		do { *(disp_base + (a - DISP_REG_BASE)) = (d); udelay(10); }while(0)
#define tve0_read(a)		*(tve0_base + (a))
#define tve0_write(a, d)	do { *(tve0_base + (a)) = (d); udelay(10); }while(0)
#define tve1_read(a)		*(tve1_base + (a))
#define tve1_write(a, d)	do { *(tve1_base + (a)) = (d); udelay(10); }while(0)
#define clk_read(a)		*(clk_base + (a - REG_CLK_BASE))
#define clk_write(a, d)	do { *(clk_base + (a - REG_CLK_BASE)) = (d); udelay(10); }while(0)
#define clk_read32(a)		*(clk_base + (a - REG_CLK_BASE))
#define clk_write32(a, d)	do { *(clk_base + (a - REG_CLK_BASE)) = (d); udelay(10); }while(0)
#endif

#define REG_MAP_ADDR(x)	((((x)>>16) == 0x4180)  ? (disp_base+((x)&0xffff)) :  \
                               ((((x)>>12) == 0x10160) ? (tve1_base+((x)&0xfff)) :      \
			       ((((x)>>12) == 0x10168) ? (tve0_base+((x)&0xfff)) :  \
			       ((((x)>>12) == 0x10171) ? (clk_base+((x)&0xfff)) : 0 ))))

#define df_write8(a,v)	writeb(v,REG_MAP_ADDR(a)) 
#define df_write16(a,v)	writew(v,REG_MAP_ADDR(a))
#define df_write(a,v)	writel(v,REG_MAP_ADDR(a))
#define df_read8(a)	readb(REG_MAP_ADDR(a))
#define df_read16(a)	readw(REG_MAP_ADDR(a))
#define df_read(a)	readl(REG_MAP_ADDR(a))

DEFINE_SPINLOCK(orion15_df_lock);

typedef struct df_rect
{
        int left;
        int right;
        int top;
        int bottom;
}df_rect;

struct df_output_pos {
	struct df_rect src;
	struct df_rect dst;
}df_output_pos;

//2 GFX
typedef struct
{
	char *ImgFilePath;
	unsigned int SrcWidth     ;
	unsigned int SrcHeight    ;
	DF_GFX_COLOR_FORMAT ImgColorFmt;
} COLOR_IMG_PARA;

unsigned int Is_TTX = 0;
/************************************* FUNCTION *****************************************/
#if 1
unsigned int DF_Read(unsigned int addr)
{
	return df_read(addr);
}
void DF_Write(unsigned int addr, unsigned int data)
{
	df_write(addr, data);
	return;
}
unsigned char TVE_Read(unsigned int tveid, unsigned int addr)
{
	if(tveid == 0)
		return df_read8(0x10168000+addr);
	else if(tveid == 1)
		return df_read8(0x10160000+addr);
	else
		return -1;
}
void TVE_Write(unsigned int tveid, unsigned int addr, unsigned char data)
{
	if(tveid == 0)
		df_write8((0x10168000+addr), data);
	else if(tveid == 1)
		df_write8((0x10160000+addr), data);
	else
		return;

	return;
}
unsigned short CLK_Read(unsigned int addr)
{
	return df_read16(addr);
}
void CLK_Write(unsigned int addr, unsigned short data)
{
	df_write16(addr, data);
	return;
}
#endif

 
//1PIN MUX
//---------------------------------PIN MUX Description-------------------------------------------------------------
//pin_mux0
//---------------------------------------------------------------------------------------------------------------
//Bits Access    Name                                Description
//---------------------------------------------------------------------------------------------------------------
//15:14                                              Reserved
//13      RW     VIB Input Enable                    0:   Output            1: Input                  
//12      RW     Digital Display output configure    0:   OutIF1            1: OUIF0
//11:10   RW     Digital Display output Interface    0,1: Display DDR Data  2: YCbCr422       3: RGB/YCbCr444
//6       R                                          0:   GPIO[12]          1: PCMCIA RESET_N
//5       RW                                         0:   GPIO[14]          1: PATA RESET_N
// 4       RW     PATA_MODE                           0:   Disable PATA DMA mode 1: Enable PATA DMA mode(EBI access not allowed)
// 3:1     -                                          Reserved
//0       RW     VID_DAC0_CFG                        0: Video DAC0 output CVBS/S-Video 1: Video DAC0 output YPbPr/RGB

//---------------------------------------------------------------------------------------------------------------
//pin_mux1
//---------------------------------------------------------------------------------------------------------------
//Bits   Access  Name                                Description
//15:13   -      reserved                            Reserved
//12      RW                                         0: Digital Video Output Disable 1: Digital Video Output Enable
//11:8    -      reserved                            Reserved
//7       RW                                         0: MII Interface  1: Modem Handshaking Signals
//6       RW                                         0: Disable IR mode for third UART 1: Enable IR mode for third UART
//5       RW                                         0: GPIO[6], GPIO[3] 1: TX and RX for third UART
// 4       RW                                         0: GPIO[1], GPIO[0] 1: I2C_SCL2, I2C_SDA2
// 3: 2     RW     VID_DAC1_CFG                        1,3:TVE1 YPbPr 2:TVE0 RGB 0:TVE0 YPbPr
// 2:0     RW     AUD_JITTER_HI                       NEW_ERROR_HI[2:0]
void __pin_mux0_write(unsigned int value)
{
  df_write(IDS_OPROC_LOCTRL, value);
}

void __pin_mux1_write(unsigned int value)
{
  df_write(IDS_OPROC_HICTRL, value);
}

unsigned int __pin_mux0_read(void)
{
  return (df_read(IDS_OPROC_LOCTRL));
}

unsigned int __pin_mux1_read(void)
{
  return (df_read(IDS_OPROC_HICTRL));
}

/*
  Description : This function used to set digital output pin mux
      
  paramter:
  OutIFID:     0- Output IF  #0  !0- Output IF #1
  FmtVal:      0- YCbCr422     1- YCbCr444 /RGB      2- DDR    

*/
void DFSetDigitalOutFmt(unsigned int OutIFID,unsigned int FmtVal)
{
	unsigned int RegVal;
	
	OutIFID =!(!OutIFID);
	RegVal=__pin_mux0_read();

	if(OutIFID){
		RegVal &=0xefff;
	}
	else{
		RegVal |=0x1000;
	}

	switch(FmtVal)
	{
		case 0:
			RegVal &=0xfbff;
			RegVal |=0x800;
			break;
		case 1:
			RegVal &=0xf3ff;
			RegVal |=0xc00;
			
			break;
		default:
			RegVal &=0xf3ff;
			break;
	}
	__pin_mux0_write(RegVal);
}

/*
  when DACID=0 (Only for TVE1):
      FmtID=0   CVBS & SVideo
	  others    YPbPr

  When DACID=1 (For TVE0 & TVE1):
  
      FmtID=0   YPbPr
	  FmtID=1   RGB
	  Others    YPbPr (From TVE1)
*/
void DFSetAnalogOutFmt(unsigned int DACID,unsigned int FmtID)
{
	unsigned int tempval = 0;
	unsigned int RegVal = 0;
	DACID=!(!DACID);

	DEBUG_PRINTF("Dacid = %d\n",DACID);

	if(DACID)
	{
		RegVal=df_read(IDS_OPROC_HICTRL);
		tempval = RegVal;
		switch(FmtID)
		{
			case 0:
				RegVal &=0xfff3;
				break;
			case 1:
				RegVal |=0x8;
				RegVal &=0xfffb;
				break;
			default:
				RegVal |=0xc;
				break;
		}
		df_write(IDS_OPROC_HICTRL, RegVal);
		if((RegVal&0x4) != (tempval&0x4))
		{
			__csdrv_aud_set_freq(g_audio_currnet_freq);
			DEBUG_PRINTF("DF : audio sample rate %d\n",g_audio_currnet_freq);
		}
	}
	else
	{
		RegVal=__pin_mux0_read();
		if(FmtID==0)
		{
			RegVal &=0xfffe;
		}
		else
		{
			RegVal |=0x1;
		}
		__pin_mux0_write(RegVal);
	}
}


static unsigned char __tve0_reg_read(unsigned char addr_base)
{
	unsigned char data_rd;
	unsigned int addr ;
	addr = TVE0_REG_BASE +(addr_base);
	data_rd = df_read8(addr);
    	return data_rd;
}

static void _tve0_reg_write(unsigned char addr_base, unsigned char data)
{
	unsigned char data_rd;
	unsigned int addr ;
	addr = TVE0_REG_BASE +(addr_base);

	df_write8(addr,data);
	data_rd = df_read8(addr);

    if(data_rd != data)
	{
  	  DEBUG_PRINTF("TVE0: Reg Write ERROR: addr_base = %02x, wr_data =%08x,rd_data = %08x\n",
  	                                 addr_base,data,data_rd);
	}
}

// 1,1 HD, Progressive
// 0,1 SD, Progressive
// 0,0 SD, Interlacer
void __setTVEncoderClk_TVE0(int IsHD, int IsProgressive)
{
	unsigned short FreqCtlValue = 0;
	unsigned short TVEPLLValue  = 0;

	

	FreqCtlValue = CLK_Read(REG_CLK_FREQ_CTRL);
	FreqCtlValue  &= (~(1 << 3)); //Disable the TVE Clock
	CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue);
	udelay(1);

	//Set TVE1 clk pll value
	if (IsHD == 1){
		TVEPLLValue = 0x4621; 
	}
	else if(IsHD == 2){//VGA
		TVEPLLValue = IsProgressive;
		IsProgressive = 1;
		IsHD = 0;
	}
	else{
		TVEPLLValue = 0x8618;
	}
	CLK_Write(IDS_PCTL_LO, TVEPLLValue);
	//Delay for A while
	udelay(100);

	//Enable the TVE Clk sys REG_CLK_FREQ_CTRL[3]
	FreqCtlValue |= (0x1 << 3);
	CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue); 
	udelay(1);
	
	FreqCtlValue = CLK_Read(REG_CLK_FREQ_CTRL);
	FreqCtlValue &= (~( 0x3 << 4));
	CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue);
	udelay(1);
	
	if (!IsHD && IsProgressive)
	{
		FreqCtlValue |= (0x2 << 4);
	}
	else // HD and SD_i share this branch
	{
		FreqCtlValue |= (0x1 << 4);
	}
	CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue);
	//Read Back to Check
	{
		unsigned short ReadValue = 0;
		ReadValue = CLK_Read( REG_CLK_FREQ_CTRL);
		if(FreqCtlValue != ReadValue){
			DEBUG_PRINTF("REG_CLK_FREQ_CTRL expect value 0x%x , current value 0x%x\n",FreqCtlValue,ReadValue);
		}
	}

	return;
}

void TVE0SetClk(TVOUT_MODE DispFormat , int EnableColorBar)
{
	int VIDEO_TYPE  = TVOUT_MODE_576I;
	int IsHD = 0;
	int IsProgressive = 0;

	//Map the Display Format Value to TVE0 Format Value
	switch (DispFormat)
	{
		case TVOUT_MODE_SECAM:
			VIDEO_TYPE = TYPE_SECAM;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_PAL_N:
			VIDEO_TYPE = TYPE_PAL_N;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_PAL_CN:
			VIDEO_TYPE = TYPE_PAL_CN;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_576I:
			VIDEO_TYPE = TYPE_PAL;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_480I:
			VIDEO_TYPE = TYPE_NTSC;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_PAL_M:
			VIDEO_TYPE = TYPE_PAL_M;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_1080I30:
		case TVOUT_MODE_1080I25:
			VIDEO_TYPE = TYPE_1080i;
			IsHD = 1;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_720P50:
			VIDEO_TYPE = TYPE_720p50;
			IsHD = 1;
			IsProgressive = 1;
			break;
		case TVOUT_MODE_720P60: 
		case TVOUT_MODE_1080P24:
		case TVOUT_MODE_1080P25:
		case TVOUT_MODE_1080P30:
			VIDEO_TYPE = TYPE_720p60;
			IsHD = 1;
			IsProgressive = 1;
			break;
		case TVOUT_MODE_480P:
		case TVOUT_MODE_576P:
			VIDEO_TYPE = TYPE_480p; //???
			IsHD = 0;
			IsProgressive = 1;
			break;
		case TVOUT_RGB_640X480_60FPS:
			IsHD = 2;//VGA
			IsProgressive = 0x881e;
			break;
		case TVOUT_RGB_800X600_60FPS:
			IsHD = 2;//VGA
			IsProgressive = 0x882f;
			break;
		case TVOUT_RGB_800X600_72FPS:
			IsHD = 2;//VGA
			IsProgressive = 0x883b;
			break;
		case TVOUT_RGB_1024X768_60FPS:
			IsHD = 2;//VGA
			IsProgressive = 0x863a;
			break;
		case TVOUT_RGB_1280X1024_50FPS:
			IsHD = 2;//VGA
			IsProgressive = 0x886a;
			break;
		case TVOUT_RGB_1600X1000_60FPS:
			IsHD = 2;//VGA
			IsProgressive = 0x868f;//1600x1200 0x868f;
			break;
       	case TVOUT_RGB_1280X1024_60FPS:
			IsHD = 2;//VGA
			IsProgressive = 0x8660;
			break;
		case TVOUT_RGB_1280X720_60FPS:
			IsHD = 2;//VGA
			IsProgressive = 0x8858;
			break;
		case TVOUT_RGB_848X480_60FPS:
			IsHD = 2;//VGA
			//IsProgressive = 0x8825;
			IsProgressive = 0x8828;
			break;
		case TVOUT_RGB_800X480_60FPS:
			IsHD = 2;//VGA
			IsProgressive = 0x8823;
			break;
		default:
			VIDEO_TYPE = TYPE_PAL;
			IsHD = 0;
			IsProgressive = 0;
			break;
	}
	//Set Clk input frequecy
	__setTVEncoderClk_TVE0(IsHD, IsProgressive);
	return;
}

int InitTVE0Raw(TVOUT_MODE DispFormat , int EnableColorBar)
{
	int i = 0;
	int VIDEO_TYPE  = TVOUT_MODE_576I;
	int IsHD = 0;
	int IsProgressive = 0;

	//Map the Display Format Value to TVE0 Format Value
	switch (DispFormat)
	{
		case TVOUT_MODE_SECAM:
			VIDEO_TYPE = TYPE_SECAM;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_PAL_N:
			VIDEO_TYPE = TYPE_PAL_N;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_PAL_M:
			VIDEO_TYPE = TYPE_PAL_M;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_PAL_CN:
			VIDEO_TYPE = TYPE_PAL_CN;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_576I:
			VIDEO_TYPE = TYPE_PAL;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_480I:
			VIDEO_TYPE = TYPE_NTSC;
			IsHD = 0;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_1080I30:
		case TVOUT_MODE_1080I25:
			VIDEO_TYPE = TYPE_1080i;
			IsHD = 1;
			IsProgressive = 0;
			break;
		case TVOUT_MODE_720P60: 
			VIDEO_TYPE = TYPE_720p60;
			IsHD = 1;
			IsProgressive = 1;
			break;
		case TVOUT_MODE_720P50:
			VIDEO_TYPE = TYPE_720p50;
			IsHD = 1;
			IsProgressive = 1;
			break;
		case TVOUT_MODE_480P:
		case TVOUT_MODE_576P:
			VIDEO_TYPE = TYPE_480p; //???
			IsHD = 0;
			IsProgressive = 1;
			break;
		case TVOUT_MODE_1080P24:
		case TVOUT_MODE_1080P25:
		case TVOUT_MODE_1080P30:
			return 0;
		case TVOUT_RGB_640X480_60FPS://duliguo
			VIDEO_TYPE = TYPE_VGA;
			break;
		default:
			VIDEO_TYPE = TYPE_PAL;
			IsHD = 0;
			IsProgressive = 0;
			break;
	}

	//Hold TVE0 Timing
	_tve0_reg_write(TVE0_REG[0][REG_ADDR],0x1);
	
	DEBUG_PRINTF("[TVE0: INFO]probe REV_ID ..%d\n",TVE0_REG[1][REG_ADDR]);

	for (i=2; i<TVE0_REG_NUM; i++)
	{
		DEBUG_PRINTF("TVE0: ADDR=%03x,WriteDATA=%03x\n",TVE0_REG[i][REG_ADDR],TVE0_REG[i][VIDEO_TYPE]);
		_tve0_reg_write(TVE0_REG[i][REG_ADDR],TVE0_REG[i][VIDEO_TYPE]);
	}

	if (EnableColorBar)
	{
		unsigned char  RegValue = 0;
		RegValue = __tve0_reg_read(0x2);
		RegValue |= (1 << 5);
		_tve0_reg_write(0x2,RegValue);
	}
	_tve0_reg_write(TVE0_REG[0][REG_ADDR],0x0);
	return 0;
}

static unsigned char __tve1_reg_read(unsigned char addr_base)
{
	unsigned char data_rd;
	unsigned int addr ;
	addr = TVE1_REG_BASE +(addr_base);
	data_rd = df_read8(addr);
    	return data_rd;
}

static void _tve1_reg_write(unsigned char addr_base, unsigned char data)
{
	unsigned char data_rd;
	unsigned int addr ;
	addr = TVE1_REG_BASE +(addr_base);

//printascii("333333333333\n");
	df_write8(addr,data);
//printascii("444444444444\n");
	data_rd = df_read8(addr);
//printascii("555555555555\n");
	if(data_rd != data)
	{
		  DEBUG_PRINTF("TVE1: Reg Write ERROR: addr_base = %02x, wr_data =%08x,rd_data = %08x\n",
		                                 addr_base,data,data_rd);
	}
//printascii("66666666666\n");
}

//IsHd IsP
// 1,  1 :HD, Progressive
// 0,  1 :SD, Progressive
// 0,  0 :SD, Interlacer
// Clock Source
// 1: PLL_SYS
// 0: PLL_TVE
void __setTVEncoderClk_TVE1(int IsHD, int IsProgressive, int ClockSrc)
{
	unsigned short FreqCtlValue = 0;
	unsigned short TVEPLLValue  = 0;
	unsigned short temp         = 0;
	// Under PLL_SYS, the TVE_PLL will be configure in SetTVEncoderClk_TVE0()
	if (!ClockSrc){
		FreqCtlValue = CLK_Read(REG_CLK_FREQ_CTRL);
		FreqCtlValue  &= (~(1 << 3)); //Disable the TVE Clock
		CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue);
		//Delay for A while
		udelay(1000);
		//Set TVE1 clk pll value
		if (IsHD)
		{
			TVEPLLValue = 0x4621;
		}
		else
		{
			TVEPLLValue = 0x8618;
		}
		CLK_Write(IDS_PCTL_LO, TVEPLLValue);
		//Delay for A while
		udelay(1000);

		//Enable the TVE Clk sys REG_CLK_FREQ_CTRL[3]
		FreqCtlValue |= (1 << 3);
		CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue); 
	}
	// tve1_clk0 ~ tve1_clk5 select: tve1_clk0_sel==freq_ctrl[11,10]
	// (default:freq_ctrl =0x031a)
	// default:tve1_clk0_sel=2'b00,  clock0-5 are gating 
	// 2'b01 clock0-5 the from PLL_SYS/4
	// 2'b10 clock0~5 from the o_plltve
	// Set tve1_clk0_sel switch to PLLTVE
	// [11:10]=2'b10,
	FreqCtlValue = CLK_Read(REG_CLK_FREQ_CTRL);
	DEBUG_PRINTF("1 gating TVE1 REG_CLK_FREQ_CTRL=%x\n",FreqCtlValue);
	temp = ~(0x3 << 10);
	DEBUG_PRINTF("temp =%x\n",temp);
	//FreqCtlValue  &= (~(0x3 << 10)); //gating the tve1_clk0_sel Clock
	FreqCtlValue  &= temp; //gating the tve1_clk0_sel Clock
	CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue); 
	FreqCtlValue = CLK_Read(REG_CLK_FREQ_CTRL);
	DEBUG_PRINTF("2 gating TVE1 REG_CLK_FREQ_CTRL=%x\n",FreqCtlValue);

	if(ClockSrc){ //PLL_SYS
	    FreqCtlValue |= (0x1 << 10); //[11:10]=2'b01
	    CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue);
	    udelay(1000);
	} else {	//PLL_TVE
	    FreqCtlValue |= (0x1 << 11); //[11:10]=2'b10
	    CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue);
	    udelay(1000);
	}

	//Set TVE1_CLK6 gating:
	//[13:12]=2'b00,
	FreqCtlValue &= (~( 0x3 << 12));
	CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue);

	if (!IsHD && IsProgressive)
	{
		FreqCtlValue |= (0x2 << 12);
	}
	else // HD and SD_i share this branch
	{
		FreqCtlValue |= (0x1 << 12);
	}
	CLK_Write(REG_CLK_FREQ_CTRL, FreqCtlValue);
	//Read Back to Check
	{
		unsigned short ReadValue = 0;
		ReadValue = CLK_Read( REG_CLK_FREQ_CTRL);
		if(FreqCtlValue != ReadValue){
			DEBUG_PRINTF("REG_CLK_FREQ_CTRL expect value 0x%x , current value 0x%x\n",FreqCtlValue,ReadValue);
		}
	}
	FreqCtlValue = CLK_Read(REG_CLK_FREQ_CTRL);
	DEBUG_PRINTF("clock TVE1 REG_CLK_FREQ_CTRL=%x\n",FreqCtlValue);
	//For Test
	//Enable CCIR 656 Clk

	return;
}

void TVE1SetClk(TVOUT_MODE DispFormat , int EnableColorBar, int IsSysPll)
{
    int VIDEO_TYPE  = TVOUT_MODE_576I;
    int IsHD = 0;
    int IsProgressive = 0;

    //Map the Display Format Value to TVE1 Format Value
    switch (DispFormat)
    {
        case TVOUT_MODE_1080I30:
        case TVOUT_MODE_1080I25:
            VIDEO_TYPE = TYPE_1080i;
            IsHD = 1;
            IsProgressive = 0;
            break;
        case TVOUT_MODE_720P60: 
            VIDEO_TYPE = TYPE_720p60;
            IsHD = 1;
            IsProgressive = 1;
            break;
        case TVOUT_MODE_720P50:
            VIDEO_TYPE = TYPE_720p50;
            IsHD = 1;
            IsProgressive = 1;
            break;
        case TVOUT_MODE_480P:
            VIDEO_TYPE = TYPE_480p;
            IsHD = 0;
            IsProgressive = 1;
            break;
        case TVOUT_MODE_576P:
            VIDEO_TYPE = TYPE_576p; 
            IsHD = 0;
            IsProgressive = 1;
            break;
        case TVOUT_MODE_SECAM:
		VIDEO_TYPE = TYPE_SECAM;
		IsHD = 0;
		IsProgressive = 0;
		break;
	case TVOUT_MODE_PAL_N:
		VIDEO_TYPE = TYPE_PAL_N;
		IsHD = 0;
		IsProgressive = 0;
		break;
	case TVOUT_MODE_PAL_CN:
		VIDEO_TYPE = TYPE_PAL_CN;
		IsHD = 0;
		IsProgressive = 0;
		break;
	case TVOUT_MODE_576I:
		VIDEO_TYPE = TYPE_PAL;
		IsHD = 0;
		IsProgressive = 0;
		break;
	case TVOUT_MODE_480I:
		VIDEO_TYPE = TYPE_NTSC;
		IsHD = 0;
		IsProgressive = 0;
		break;
	case TVOUT_MODE_PAL_M:
		VIDEO_TYPE = TYPE_PAL_M;
		IsHD = 0;
		IsProgressive = 0;
		break;
        default:
            VIDEO_TYPE = TYPE_PAL;
            IsHD = 0;
            IsProgressive = 0;
            break;
    }
    //Set Clk input frequecy
    __setTVEncoderClk_TVE1(IsHD, IsProgressive, IsSysPll);
    return;
}

int InitTVE1Raw( TVOUT_MODE DispFormat , int EnableColorBar, int IsSysPll)
{
    int i = 0;
    int VIDEO_TYPE  = TVOUT_MODE_576I;
    int IsHD = 0;
    int IsProgressive = 0;

    //Map the Display Format Value to TVE1 Format Value
    switch (DispFormat)
    {
	    case TVOUT_MODE_SECAM:
		    VIDEO_TYPE = TYPE_SECAM;
		    IsHD = 0;
		    IsProgressive = 0;
		    break;
	    case TVOUT_MODE_PAL_N:
		    VIDEO_TYPE = TYPE_PAL_N;
		    IsHD = 0;
		    IsProgressive = 0;
		    break;
	    case TVOUT_MODE_PAL_M:
		    VIDEO_TYPE = TYPE_PAL_M;
		    IsHD = 0;
		    IsProgressive = 0;
		    break;
	    case TVOUT_MODE_PAL_CN:
		    VIDEO_TYPE = TYPE_PAL_CN;
		    IsHD = 0;
		    IsProgressive = 0;
		    break;
	    case TVOUT_MODE_576I:
		    VIDEO_TYPE = TYPE_PAL;
		    IsHD = 0;
		    IsProgressive = 0;
		    break;
	    case TVOUT_MODE_480I:
		    VIDEO_TYPE = TYPE_NTSC;
		    IsHD = 0;
		    IsProgressive = 0;
		    break;
	    case TVOUT_MODE_1080I30:
	    case TVOUT_MODE_1080I25:
		    VIDEO_TYPE = TYPE_1080i;
		    IsHD = 1;
		    IsProgressive = 0;
		    break;
	    case TVOUT_MODE_720P60: 
		    VIDEO_TYPE = TYPE_720p60;
		    IsHD = 1;
		    IsProgressive = 1;
		    break;
	    case TVOUT_MODE_720P50:
		    VIDEO_TYPE = TYPE_720p50;
		    IsHD = 1;
		    IsProgressive = 1;
		    break;
	    case TVOUT_MODE_480P:
	    case TVOUT_MODE_576P:
		    VIDEO_TYPE = TYPE_480p; //???
		    IsHD = 0;
		    IsProgressive = 1;
		    break;
	    case TVOUT_MODE_1080P24:
	    case TVOUT_MODE_1080P25:
	    case TVOUT_MODE_1080P30:
		    return 0;
	    case TVOUT_RGB_640X480_60FPS://duliguo
		    VIDEO_TYPE = TYPE_VGA;
		    break;
	    default:
		    VIDEO_TYPE = TYPE_PAL;
		    IsHD = 0;
		    IsProgressive = 0;
		    break;
    }
//printascii("1111111111\n");
    //Hold TVE1 Timing
    _tve1_reg_write(TVE1_REG[0][REG_ADDR],0x1);
    DEBUG_PRINTF("[TVE1: INFO]probe REV_ID ..%d\n",TVE1_REG[1][REG_ADDR]);
//printascii("222222222222\n");

    for (i=2; i<TVE1_REG_NUM; i++)
    {
    DEBUG_PRINTF("TVE1: ADDR=%03x,WriteDATA=%03x\n",TVE1_REG[i][REG_ADDR],TVE1_REG[i][VIDEO_TYPE]);
    _tve1_reg_write(TVE1_REG[i][REG_ADDR],TVE1_REG[i][VIDEO_TYPE]);
    }

    if (EnableColorBar)
    {
        unsigned char RegValue = 0;
        RegValue = __tve1_reg_read(0x2);
        RegValue |= (1 << 5);
        _tve1_reg_write(0x2,RegValue);
    }
    _tve1_reg_write(TVE1_REG[0][REG_ADDR],0x0);
    
    return 0;
}

//1DF
static df_out_if_timing outputFomatInfo[] =
{
	{1, 720, 480, 30, 27000,0,1,1,1,1,0,0,1,0,0,1, 858,138, 525,21, 261,284, 524,3,265, 19, 81, 3, 6,265,268, DISP_YUV_NTSC,            "DISP_YUV_NTSC"},
	{1, 1280, 720, 60, 74250,1,0,0,0,0,0,0,0,1,1,1,1650,370, 750,25, 745, 25, 745,0,  0,110,150, 0, 5,  0,  5, DISP_YUV_720P_60FPS,      "DISP_YUV_720P_60FPS"},
	{1, 1280, 720, 50, 74250,1,0,0,0,0,0,0,0,1,1,1,1980,700, 750,25, 745, 25, 745,0,  0,440,480, 0, 5,  0,  5, DISP_YUV_720P_50FPS,      "DISP_YUV_720P_50FPS"},
	{1, 1920,1080, 30, 74250,1,1,0,0,1,0,0,1,1,1,1,2200,280,1125,20, 560,583,1123,0,562, 88,132, 0, 5,562,567, DISP_YUV_1080I_30FPS,     "DISP_YUV_1080I_30FPS"},
	{1, 1920,1080, 25, 74250,1,1,0,0,1,0,0,1,1,1,1,2640,720,1125,20, 560,583,1123,0,562,528,572, 0, 5,562,567, DISP_YUV_1080I_25FPS,     "DISP_YUV_1080I_25FPS"},
	{1, 720, 576, 50, 27000,0,1,1,1,0,1,0,1,0,0,1, 864,144, 625,22, 310,335, 623,0,312, 12, 75, 0, 3,312,315, DISP_YUV_PAL,             "DISP_YUV_PAL"},
	{1, 720, 480, 60, 27000,0,0,0,0,0,0,0,0,0,0,1, 858,138, 525,42, 522, 42, 522,0,  0, 16, 78, 6,12,  6, 12, DISP_YUV_480P,            "DISP_YUV_480P"},
	{1, 720, 576, 50, 27000,0,0,0,0,0,0,0,0,0,0,1 ,864,144, 625,44, 620, 44, 620,0,  0, 12, 76, 0, 5,  0,  5, DISP_YUV_576P,            "DISP_YUV_576P"},
	{1, 1920,1080, 30, 74250,1,0,0,0,0,0,0,0,1,1,1,2200,280,1125,41,1121, 41,1121,0,  0, 88,132, 0, 5,  0,  5, DISP_YUV_1080P_30FPS,     "DISP_YUV_1080P_30FPS"},
	{1, 1920,1080, 25, 74250,1,0,0,0,0,0,0,0,1,1,1,2640,720,1125,41,1121, 41,1121,0,  0,528,572, 0, 5,  0,  5, DISP_YUV_1080P_25FPS,     "DISP_YUV_1080P_25FPS"},
	{1, 1920,1080, 60,148500,1,0,0,0,0,0,0,0,1,1,1,2200,280,1125,41,1121, 41,1121,0,  0, 88,132, 0, 5,  0,  5, DISP_YUV_1080P_60FPS,     "DISP_YUV_1080P_60FPS"},
	{1, 1920,1080, 50,148500,1,0,0,0,0,0,0,0,1,1,1,2640,720,1125,41,1121, 41,1121,0,  0,528,572, 0, 5,  0,  5, DISP_YUV_1080P_50FPS,     "DISP_YUV_1080P_50FPS"},
	{0, 640, 480, 60, 25170,0,0,0,0,0,0,0,0,0,0,1, 800,160, 525,45,   0, 45,   0,0,  0, 16,112,11,13, 11, 13, DISP_RGB_640X480_60FPS,   "DISP_RGB_640X480_60FPS"},
	{0, 800, 600, 60, 40000,0,0,0,0,0,0,0,0,0,0,1,1056,256, 628,28,   0, 28,   0,0,  0, 40,168, 1, 5,  1,  5, DISP_RGB_800X600_60FPS,   "DISP_RGB_800X600_60FPS"},
	{0, 800, 600, 72, 50000,0,0,0,0,0,0,0,0,0,0,1,1040,240, 666,64, 664, 64, 664,0,  0, 56,176,35,41, 35, 41, DISP_RGB_800X600_72FPS,   "DISP_RGB_800X600_72FPS"},
	{0, 1024, 768, 60, 65000,1,0,0,0,0,0,0,0,0,0,1,1344,320, 806,38,   0, 38,   0,0,  0, 24,160, 3, 9,  3,  9, DISP_RGB_1024X768_60FPS,  "DISP_RGB_1024X768_60FPS"},
	{0, 1280,1024, 50, 89375,1,0,0,0,0,0,0,0,0,0,1,1696,416,1054,30,   0, 30,   0,0,  0, 72,208, 1, 4,  1,  4, DISP_RGB_1280X1024_50FPS, "DISP_RGB_1280X1024_50FPS"},
	{0, 1600,1200, 60,160875,1,0,0,0,0,0,0,0,0,0,1,2160,560,1242,42,   0, 42,   0,0,  0, 104,280, 1, 4,  1,  4, DISP_RGB_1600X1000_60FPS, "DISP_RGB_1600X1000_60FPS"},
	//{0, 1600,1000, 60,133125,1,0,0,0,0,0,0,0,0,0,1,2144,544,1035,35,   0, 35,   0,0,  0, 88,256, 1, 4,  1,  4, DISP_RGB_1600X1000_60FPS, "DISP_RGB_1600X1000_60FPS"},
	{0, 1280,1024, 60, 89000,1,0,0,0,0,0,0,0,0,0,1,1408,128,1054,30,   0, 30,   0,0,  0, 32,64, 1, 4,  1,  4, DISP_RGB_1280X1024_60FPS, "DISP_RGB_1280X1024_60FPS"},
	{1, 1920,1080, 24, 74250,1,0,0,0,0,0,0,0,1,1,1,2750,830,1125,41,1121, 41,1121,0,  0,638,682, 0, 5,  0,  5, DISP_YUV_1080P_24FPS,     "DISP_YUV_1080P_24FPS"},
	{0, 1024, 768, 70, 75000,1,0,0,0,0,0,0,0,0,0,1,1328,304, 806,38,   0, 38,   0,0,  0, 24,160, 3, 9,  3,  9, DISP_RGB_1024X768_70FPS,  "DISP_RGB_1024X768_70FPS"},
	{0, 1280, 720, 60, 74375,1,0,0,0,0,0,0,0,0,0,1,1664,384, 746,26,   0, 26,   0,0,  0, 56,192, 1, 4,  1,  4, DISP_RGB_1280x720_60FPS, "DISP_RGB_1280x720_60FPS"},
//	{0, 848, 480, 60, 31500,1,0,0,0,0,0,0,0,0,0,1,1056,208, 497,17,   0, 17,   0,0,  0, 88,192, 3, 16,  3,  16, DISP_RGB_848X480_60FPS, "DISP_RGB_848x480_60FPS"},
	{0, 848, 480, 60, 33750,1,0,0,0,0,0,0,0,0,0,1,1088,240, 517,37,   0, 37,   0,0,  0, 16,128, 6, 14,  6,  14, DISP_RGB_848X480_60FPS, "DISP_RGB_848x480_60FPS"},
	{0, 800, 480, 60, 29500,1,0,0,0,0,0,0,0,0,0,1,992,192, 500,20,   0, 20,   0,0,  0, 24,96, 3, 10,  3,  10, DISP_RGB_800X480_60FPS, "DISP_RGB_848x480_60FPS"},

};

static int dfreg_ctrl[8] =
{
	DISP_UPDATE_REG,DISP_STATUS,DISP_OUTIF1_INT_CLEAR,DISP_OUTIF2_INT_CLEAR,DISP_OUTIF1_ERR_CLEAR,
 	DISP_OUTIF2_ERR_CLEAR,DISP_SCA_COEF_IDX,DISP_SCA_COEF_DATA
};
static int dfreg_gfx[2][13] =
{
	{DISP_GFX1_CTRL,DISP_GFX1_FORMAT,DISP_GFX1_ALPHA_CTRL,DISP_GFX1_KEY_RED,DISP_GFX1_KEY_BLUE,DISP_GFX1_KEY_GREEN,
	DISP_GFX1_BUF_START,DISP_GFX1_LINE_PITCH,DISP_GFX1_X_POSITON,DISP_GFX1_Y_POSITON,DISP_GFX1_SCL_X_POSITON,
	DISP_GFX1_CLUT_ADDR,DISP_GFX1_CLUT_DATA
	},
	{DISP_GFX2_CTRL,DISP_GFX2_FORMAT,DISP_GFX2_ALPHA_CTRL,DISP_GFX2_KEY_RED,DISP_GFX2_KEY_BLUE,DISP_GFX2_KEY_GREEN,
	DISP_GFX2_BUF_START,DISP_GFX2_LINE_PITCH,DISP_GFX2_X_POSITON,DISP_GFX2_Y_POSITON,DISP_GFX2_SCL_X_POSITON,
	DISP_GFX2_CLUT_ADDR,DISP_GFX2_CLUT_DATA
	},
};
static int dfreg_video[2][20] =
{
	{DISP_VIDEO1_CTRL,DISP_VIDEO1_ALPHA_CTRL,DISP_VIDEO1_KEY_LUMA,DISP_VIDEO1_X_POSITON,DISP_VIDEO1_Y_POSITON,
	DISP_VIDEO1_SRC_X_CROP,DISP_VIDEO1_SRC_Y_CROP,DISP_VIDEO1_CM_COEF0_012,DISP_VIDEO1_CM_COEF0_3,
	DISP_VIDEO1_CM_COEF1_012,DISP_VIDEO1_CM_COEF1_3,DISP_VIDEO1_CM_COEF2_012,DISP_VIDEO1_CM_COEF2_3,
	DISP_VIDEO1_STA_IMG_SIZE,DISP_VIDEO1_STA_FRM_INFO,DISP_VIDEO1_STA_Y_TOPADDR,DISP_VIDEO1_STA_Y_BOTADDR,
	DISP_VIDEO1_STA_C_TOPADDR,DISP_VIDEO1_STA_C_BOTADDR,DISP_VIDEO1_STA_DISP_NUM
	},
	{DISP_VIDEO2_CTRL,DISP_VIDEO2_ALPHA_CTRL,DISP_VIDEO2_KEY_LUMA,DISP_VIDEO2_X_POSITON,DISP_VIDEO2_Y_POSITON,
	DISP_VIDEO2_SRC_X_CROP,DISP_VIDEO2_SRC_Y_CROP,DISP_VIDEO2_CM_COEF0_012,DISP_VIDEO2_CM_COEF0_3,
	DISP_VIDEO2_CM_COEF1_012,DISP_VIDEO2_CM_COEF1_3,DISP_VIDEO2_CM_COEF2_012,DISP_VIDEO2_CM_COEF2_3,
	DISP_VIDEO2_STA_IMG_SIZE,DISP_VIDEO2_STA_FRM_INFO,DISP_VIDEO2_STA_Y_TOPADDR,DISP_VIDEO2_STA_Y_BOTADDR,
	DISP_VIDEO2_STA_C_TOPADDR,DISP_VIDEO2_STA_C_BOTADDR,DISP_VIDEO2_STA_DISP_NUM
	},
};
static int dfreg_comp[2][3] =
{
	{DISP_COMP1_CLIP,DISP_COMP1_BACK_GROUND,DISP_COMP1_Z_ORDER
	},
	{DISP_COMP2_CLIP,DISP_COMP2_BACK_GROUND,DISP_COMP2_Z_ORDER
	},
};
static int dfreg_hd2sd[6] =
{
	DISP_HD2SD_CTRL,DISP_HD2SD_DES_SIZE,DISP_HD2SD_DES_SIZE,DISP_HD2SD_ADDR_C,DISP_HD2SD_BUF_PITCH,
	DISP_HD2SD_STATUS
};
static int dfreg_outif[2][12] =
{
	{DISP_OUTIF1_CTRL,DISP_OUTIF1_X_SIZE,DISP_OUTIF1_Y_TOTAL,DISP_OUTIF1_ACTIVE_TOP,DISP_OUTIF1_ACTIVE_BOT,
 	DISP_OUTIF1_BLANK_LEVEL,DISP_OUTIF1_CCIR_F_START,DISP_OUTIF1_HSYNC,DISP_OUTIF1_VSYNC_TOP,DISP_OUTIF1_VSYNC_BOT,
	DISP_OUTIF1_STA_DISP_SIZE,DISP_OUTIF1_STA_LINE
	},
	{DISP_OUTIF2_CTRL,DISP_OUTIF2_X_SIZE,DISP_OUTIF2_Y_TOTAL,DISP_OUTIF2_ACTIVE_TOP,DISP_OUTIF2_ACTIVE_BOT,
 	DISP_OUTIF2_BLANK_LEVEL,DISP_OUTIF2_CCIR_F_START,DISP_OUTIF2_HSYNC,DISP_OUTIF2_VSYNC_TOP,DISP_OUTIF2_VSYNC_BOT,
	DISP_OUTIF2_STA_DISP_SIZE,DISP_OUTIF1_STA_LINE
	},
};

void __df_update_start(void)
{
    df_write(dfreg_ctrl[0], 1);
}
void __df_update_end(void)
{
    df_write(dfreg_ctrl[0], 0);
}

df_out_if_timing *__df_GetOutTimming(DF_VIDEO_FORMAT Format)
{
	if((Format > NOT_STANDARD_VIDEO_MODE) && (Format < DISP_FMT_MAX))
		return &outputFomatInfo[Format];
	else
		return 0;
}

int __dfOutIFTimingCfg(df_outif_reg *Reg, DF_VIDEO_FORMAT Format)
{
	if(outputFomatInfo[Format].IsYUVorRGB == 1)
	{
		Reg->df_outif_control.bits.iClkOutSel = 7;
	}
	else if(outputFomatInfo[Format].IsYUVorRGB == 0)
	{
		Reg->df_outif_control.bits.iClkOutSel = 4;
	}

	Reg->df_outif_control.bits.is_rgb_fmt = !outputFomatInfo[Format].IsYUVorRGB;	
	Reg->df_outif_control.bits.iIsHD = outputFomatInfo[Format].iIsHD;
	Reg->df_outif_control.bits.iIsInterlaced = outputFomatInfo[Format].iIsInterlaced;
	Reg->df_outif_control.bits.iNeedRepeat = outputFomatInfo[Format].iNeedRepeat;
	Reg->df_outif_control.bits.iNeedMux = outputFomatInfo[Format].iNeedMux;
	Reg->df_outif_control.bits.iCVE5F0IsHalfLine = outputFomatInfo[Format].iCVE5F0IsHalfLine;
	Reg->df_outif_control.bits.iCVE5F1IsHalfLine = outputFomatInfo[Format].iCVE5F1IsHalfLine;
	Reg->df_outif_control.bits.iVgaF0IsHalfLine = outputFomatInfo[Format].iVgaF0IsHalfLine;
	Reg->df_outif_control.bits.iVgaF1IsHalfLine = outputFomatInfo[Format].iVgaF1IsHalfLIne;
	Reg->df_outif_control.bits.iHSyncPolarity = outputFomatInfo[Format].iHSyncPolarity;
	Reg->df_outif_control.bits.iVSyncPolarity = outputFomatInfo[Format].iVSyncPolarity;
	Reg->df_outif_control.bits.iDEPolarity = outputFomatInfo[Format].iDEPolarity;
	
	Reg->df_outif_x_size.bits.iXTotal = outputFomatInfo[Format].iXTotal;
	Reg->df_outif_x_size.bits.iXActiveStart = outputFomatInfo[Format].iXActiveStart;
	Reg->df_outif_y_size.bits.iYTotal = outputFomatInfo[Format].iYTotal;
	Reg->df_outif_active_top.bits.iYTopActiveStart = outputFomatInfo[Format].iYTopActiveStart;
	Reg->df_outif_active_top.bits.iYTopActiveEnd = outputFomatInfo[Format].iYTopActiveEnd;
	Reg->df_outif_active_bot.bits.iYBotActiveStart = outputFomatInfo[Format].iYBotActiveStart;
	Reg->df_outif_active_bot.bits.iYBotActiveEnd = outputFomatInfo[Format].iYBotActiveEnd;
	Reg->df_outif_ccir_f_start.bits.iCCIRF0Start = outputFomatInfo[Format].iCCIRF0Start;
	Reg->df_outif_ccir_f_start.bits.iCCIRF1Start = outputFomatInfo[Format].iCCIRF1Start;
	Reg->df_outif_h_sync.bits.iHSyncStart = outputFomatInfo[Format].iHSyncStart;
	Reg->df_outif_h_sync.bits.iHSyncEnd = outputFomatInfo[Format].iHSyncEnd;
	Reg->df_outif_v_sync_top.bits.iVSyncTopStart = outputFomatInfo[Format].iVSyncTopStart;
	Reg->df_outif_v_sync_top.bits.iVSyncTopEnd = outputFomatInfo[Format].iVSyncTopEnd;
	Reg->df_outif_v_sync_bot.bits.iVSyncBotStart = outputFomatInfo[Format].iVSyncBotStart;
	Reg->df_outif_v_sync_bot.bits.iVSyncBotEnd = outputFomatInfo[Format].iVSyncBotEnd;
	if (Format == DISP_YUV_PAL)
		Reg->df_outif_control.bits.iIsPal = 1;
	else
		Reg->df_outif_control.bits.iIsPal = 0;
	return Format;
}

enum _DF_SCALER_COEFF_TYPE_
{
	DF_VIDEO1_Y_HFIR_COEFF_IDX = 0,// 0: Video 1 HFIR Coeff
	DF_VIDEO1_Y_VFIR_COEFF_IDX = 1,// 1: Video 1 VFIR Coeff
	DF_VIDEO2_Y_HFIR_COEFF_IDX = 2,// 2: Video 2 HFIR Coeff
	DF_VIDEO2_Y_VFIR_COEFF_IDX = 3,// 3: Video 2 VFIR Coeff
	DF_HD2SD_Y_HFIR_COEFF_IDX  = 4,// 4: HD2SD 1 HFIR Coeff
};

enum _DF_VIDEO_SCALER_CFG_
{
	//Scaler Coeff Cfg
	DF_SCL_STEP_FRACTION_BIT_WIDTH  = 13,
	
	DF_SCL_COEFF_FRACTION_BIT_WIDTH = 10,
	DF_SCL_COEFF_INTREGER_BIT_WIDTH = 1,
	DF_SCL_COEFF_POLARITY_BIT       = 15,

	DF_SCL_WIDTH_MAX = 2048,
	DF_SCL_FIR_TAP_MAX  = 32,
	
	//Video Scaler Cfg
	DF_VIDEO_SCL_LUMA_VFIR_TAP_NUM          = 4,
	DF_VIDEO_SCL_LUMA_VFIR_PHASE_LOG2BITS   = 4,
	DF_VIDEO_SCL_LUMA_VFIR_PHASE_NUM        = 1 << DF_VIDEO_SCL_LUMA_VFIR_PHASE_LOG2BITS,
	DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM          = 4,	
	DF_VIDEO_SCL_LUMA_HFIR_PHASE_LOG2BITS   = 4,
	DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM        = 1 << DF_VIDEO_SCL_LUMA_HFIR_PHASE_LOG2BITS,
	
	DF_VIDEO_SCL_CHROMA_VFIR_TAP_NUM        = 2,
	DF_VIDEO_SCL_CHROMA_VFIR_PHASE_LOG2BITS = 4,
	DF_VIDEO_SCL_CHROMA_VFIR_PHASE_NUM      = 1 << DF_VIDEO_SCL_CHROMA_VFIR_PHASE_LOG2BITS,
	DF_VIDEO_SCL_CHROMA_HFIR_TAP_NUM        = 2,	
	DF_VIDEO_SCL_CHROMA_HFIR_PHASE_LOG2BITS = 4,
	DF_VIDEO_SCL_CHROMA_HFIR_PHASE_NUM      = 1 << DF_VIDEO_SCL_CHROMA_HFIR_PHASE_LOG2BITS,

	//HD2SD Capture Block
	DF_HD2SD_SCL_LUMA_VFIR_TAP_NUM          = 2,
	DF_HD2SD_SCL_LUMA_VFIR_PHASE_LOG2BITS   = 4,
	DF_HD2SD_SCL_LUMA_VFIR_PHASE_NUM        = 1 << DF_HD2SD_SCL_LUMA_VFIR_PHASE_LOG2BITS,
	DF_HD2SD_SCL_LUMA_HFIR_TAP_NUM          = 4,	
	DF_HD2SD_SCL_LUMA_HFIR_PHASE_LOG2BITS   = 4,
	DF_HD2SD_SCL_LUMA_HFIR_PHASE_NUM        = 1 << DF_HD2SD_SCL_LUMA_HFIR_PHASE_LOG2BITS,
	
	DF_HD2SD_SCL_CHROMA_VFIR_TAP_NUM        = 2,
	DF_HD2SD_SCL_CHROMA_VFIR_PHASE_LOG2BITS = 4,
	DF_HD2SD_SCL_CHROMA_VFIR_PHASE_NUM      = 1 << DF_HD2SD_SCL_CHROMA_VFIR_PHASE_LOG2BITS,
	DF_HD2SD_SCL_CHROMA_HFIR_TAP_NUM        = 2,	
	DF_HD2SD_SCL_CHROMA_HFIR_PHASE_LOG2BITS = 4,
	DF_HD2SD_SCL_CHROMA_HFIR_PHASE_NUM      = 1 << DF_HD2SD_SCL_CHROMA_HFIR_PHASE_LOG2BITS,

	//Gfx Horizontal Scaler
	DF_GFX_SCL_STEP_FRACTION_BIT_WIDTH  = 10,
	DF_GFX_SCL_HFIR_TAP_NUM        = 2,
	DF_GFX_SCL_HFIR_PHASE_LOG2BITS = 6,
	DF_GFX_SCL_HFIR_PHASE_NUM      = 1 << DF_GFX_SCL_HFIR_PHASE_LOG2BITS,	
};

#if defined(DEFAULT_SCALER)
static unsigned int YHFIRCoeff[DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM][DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM] =
{
	{0, 1024, 0, 0},
	{32796, 1014, 39, 32769},
	{32817, 987, 93, 32775},
	{32831, 944, 157, 32782},
	{32840, 888, 232, 32792},
	{32843, 820, 313, 32802},
	{32843, 745, 399, 32813},
	{32838, 662, 487, 32823},
	{32832, 576, 576, 32832},
	{32823, 487, 662, 32838},
	{32813, 399, 745, 32843},
	{32802, 313, 820, 32843},
	{32792, 232, 888, 32840},
	{32782, 157, 944, 32831},
	{32775, 93, 987, 32817},
	{32769, 39, 1014, 32796}
};

static unsigned int YVFIRCoeff[DF_VIDEO_SCL_LUMA_VFIR_PHASE_NUM][DF_VIDEO_SCL_LUMA_VFIR_TAP_NUM] =
{
	{0, 1024, 0, 0},
	{0, 960, 64, 0},
	{0, 896, 128, 0},
	{0, 832, 192, 0},
	{0, 768, 256, 0},
	{0, 704, 320, 0},
	{0, 640, 384, 0},
	{0, 576, 448, 0},
	{0, 512, 512, 0},
	{0, 448, 576, 0},
	{0, 384, 640, 0},
	{0, 320, 704, 0},
	{0, 256, 768, 0},
	{0, 192, 832, 0},
	{0, 128, 896, 0},
	{0, 64, 960, 0}
};
#endif

int GetDFScalerCfg(int ScalerType, int *TapNum, int *PhaseNum)
{
	switch (ScalerType)
	{
		case DF_VIDEO1_Y_HFIR_COEFF_IDX:
		case DF_VIDEO2_Y_HFIR_COEFF_IDX:
			*TapNum   = DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM;
			*PhaseNum = DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM;
			break;
		case DF_VIDEO1_Y_VFIR_COEFF_IDX:
		case DF_VIDEO2_Y_VFIR_COEFF_IDX:
			*TapNum   = DF_VIDEO_SCL_LUMA_VFIR_TAP_NUM;
			*PhaseNum = DF_VIDEO_SCL_LUMA_VFIR_PHASE_NUM;
			break;
		case DF_HD2SD_Y_HFIR_COEFF_IDX :
			*TapNum   = DF_HD2SD_SCL_LUMA_HFIR_TAP_NUM;
			*PhaseNum = DF_HD2SD_SCL_LUMA_HFIR_PHASE_NUM;
			break;
	}
	
	return ScalerType;
}

#if defined(DEFAULT_SCALER)
int DFSetScalerCoeff(int VideoId, unsigned int *Coeff, int ScalerCoefType)
{
	int TapIdx = 0;
	int PhaseIdx = 0;
	int TapNum = 0, PhaseNum  = 0;
		
	GetDFScalerCfg(ScalerCoefType, &TapNum, &PhaseNum);

	for (PhaseIdx = 0; PhaseIdx < PhaseNum; PhaseIdx++)
	{
		for (TapIdx = 0; TapIdx < TapNum; TapIdx++)
		{	
			int CoeffIdx  = 0;
			unsigned int CoeffData = 0;
			
			CoeffIdx = PhaseIdx * TapNum + TapIdx;
			CoeffData = Coeff[CoeffIdx];
			
			// Bit[ 1: 0] : Coeff Tap Idx, 0..3, Reset Value 0;
			// Bit[ 7: 2] : Reserved to be zero,
			// Bit[11: 8] : Coeff Phase Idx, 0..15, Reset Value 0;
			// Bit[15:12] : Reserved to be zero,
			// Bit[18:16] : Coeff Type: Reset Value 0;
			//              0 : Video 1 HFIR Coeff
			//              1 : Video 1 VFIR Coeff
			//              2 : Video 2 HFIR Coeff
			//              3 : Video 2 VFIR Coeff
			//              4 : HD2SD 1 HFIR Coeff
			//       *Note: Other Value will not take any Effect
			df_write(dfreg_ctrl[6], (TapIdx & 0x3) | ((PhaseIdx & 0xf) << 8) | ((ScalerCoefType & 0x7) << 16));
			
			// Bit[11: 0] : Coeff Data, Reset Value 0;
			// Bit[14:12] : Reserved to be zero;
			// Bit[   15] : Signed Bit, Reset Value 0;
			//              1: Negtive Coeff, 0: Positive Coeff
			df_write(dfreg_ctrl[7], CoeffData);
		}
	}
	
	// df_write(dfreg_ctrl[0], 0); // finishd registers update
	
	return 0;
}

int DFVideoSetDefaultCoeffRaw
(
	int VideoId,
	int SrcCropXWidth ,int DispWinXWidth,
	int SrcCropYHeight,int DispWinYHeight,
	int Is4TapOr2Tap //Vertical Only
)
{
	int rt = 0, tmp;
	int CoeffType = 0;
	
	VideoId = VideoId & 0x1;
	if (SrcCropXWidth  == 0) SrcCropXWidth  = 1024;
	if (DispWinXWidth  == 0) DispWinXWidth  = 1024;
	if (SrcCropYHeight == 0) SrcCropYHeight = 1024;
	if (DispWinYHeight == 0) DispWinYHeight = 1024;

	// why ???
	//GenFIRCoeff(YHFIRCoeff[0], 
	//	DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM, 
	//	DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM, SrcCropXWidth, DispWinXWidth);
	
	if ( SrcCropXWidth > 1920/2)
	{	
		// VFIR 2 Tap Only
//	GenFIRCoeffBilinear(YVFIRCoeff[0], 
//		DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM, 
//			DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM, SrcCropYHeight, DispWinYHeight);
		
		/* video ctrl regs, scalor VFIR tap num sel = 1 */
		tmp = df_read(dfreg_video[VideoId][0]);
		tmp |= 0x10;
		df_write(dfreg_video[VideoId][0], tmp); 
	}
	else
	{
		if (Is4TapOr2Tap)
		{
	//		GenFIRCoeff(YVFIRCoeff[0], 
	//			DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM, 
	//			DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM, SrcCropYHeight, DispWinYHeight);
		}
		else
		{
	//		GenFIRCoeffBilinear(YVFIRCoeff[0], 
	//			DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM, 
	//			DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM, SrcCropYHeight, DispWinYHeight);
		}
		
		/* video ctrl regs, scalor VFIR tap num sel = 0 */
		tmp = df_read(dfreg_video[VideoId][0]);
		tmp &= ~0x10;
		df_write(dfreg_video[VideoId][0], tmp); 
	}
	
	// Active DF DC ??
	// ... ...
	
	// Load the Coeff
	CoeffType = (VideoId == 0) ? DF_VIDEO1_Y_HFIR_COEFF_IDX : DF_VIDEO2_Y_HFIR_COEFF_IDX;
	rt = DFSetScalerCoeff(VideoId, YHFIRCoeff[0], CoeffType);
	
	CoeffType = (VideoId == 0) ? DF_VIDEO1_Y_VFIR_COEFF_IDX : DF_VIDEO2_Y_VFIR_COEFF_IDX;
	rt = DFSetScalerCoeff(VideoId, YVFIRCoeff[0], CoeffType);
	
	return rt;
}

int DFVideoSetDefaultCoeff(int VideoId)
{
	return DFVideoSetDefaultCoeffRaw
		(
			VideoId,
			1024,  1024,
			1024,  1024,
			1 //Select 4TapCoeff
		);
}
#else
//************************************************************//
// generate the Cubic convolution interpolation coeff
//************************************************************//
/* in-->Q28,  out-->Q10 */
void GenCoeff(unsigned int SrcSize, unsigned int DesSize, unsigned char CoeffType)
{
	unsigned int scaleratio;
	int phaseindex;
	int tapindex;
	int samplePos;
	int Offset = 0x30000000; // (TapNum-1)/2 --> Q3.28
	int sampleInteral = 0x400000; // 1/PhaseMax --> Q0.28
	unsigned int hi, lo;
	unsigned int reg;
	int TapNum = 0, PhaseNum  = 0;

	GetDFScalerCfg(CoeffType, &TapNum,&PhaseNum);

	if (DesSize >= SrcSize){ // scale down,should spread the interpolator kernel
		scaleratio = 0xFFFFFFFF; // scaleratio = 1; Q32 
	}
	else{
		mpa_mulu(0xFFFFFFFF, DesSize, &hi, &lo);
		scaleratio = mpa_div64(hi, lo, SrcSize);/* max width: 1920  min width: 320
		max height: 1080 min height: 240 */
	}

	for (phaseindex = 0; phaseindex < PhaseNum; phaseindex++){	
		int fCoeff[8];
		int sum = 0;
		int Coeff_32[8];

		samplePos = Offset + (sampleInteral * phaseindex); // Q3.28
		for(tapindex = 0; tapindex < TapNum; tapindex++){
			fCoeff[tapindex] = Cubic(samplePos, scaleratio); // Q0.28
			sum += fCoeff[tapindex]; // Q3.28
			samplePos = samplePos - 0x10000000;
		}

		for(tapindex = 0; tapindex < TapNum; tapindex++){
			unsigned int ufCoeff;
			unsigned int uCoeff = ABS(fCoeff[tapindex]);

			mpa_mulu(0x400, uCoeff, &hi, &lo);
			ufCoeff =  mpa_div64(hi, lo, sum);	// normalization --> Q5.10
			if (uCoeff != fCoeff[tapindex]){
			Coeff_32[tapindex] = (int)(-1 * ufCoeff);
			}
			else{
			Coeff_32[tapindex] = (int)ufCoeff;
			}
		}	

		// check sum 
		while (1) 
		{
			int i = 3; 

			sum = 0;
			for (tapindex = 0; tapindex < TapNum; tapindex++) {
				sum += Coeff_32[tapindex];  
			}
			if (sum < 0x400) {
				Coeff_32[i] += 1;
			}
			else if (sum > 0x400) {
				Coeff_32[i] -= 1;
			}
			else { //sum == 0x400
				break;
			}

			i++;
		}

		for (tapindex = 0; tapindex < TapNum; tapindex++) {

			reg = ((tapindex) <<  0) | ((phaseindex) <<  8) | ((CoeffType) << 16) ;

			if (Coeff_32[tapindex] < 0){
				Coeff_32[tapindex] = 32768 + (-1 * Coeff_32[tapindex]);
			}

			df_write(dfreg_ctrl[6], (tapindex & 0x3) | ((phaseindex & 0xf) << 8) | ((CoeffType & 0x7) << 16));
			df_write(dfreg_ctrl[7], Coeff_32[tapindex]);
			//df_write(DISP_SCA_COEF_IDX, reg);	
			//df_write(DISP_SCA_COEF_DATA, Coeff_32[tapindex]);
		}
	}
	return;
}

void DFSetVideoScalerCfg(int VideoId, unsigned int FrameWidth, unsigned int FrameHeight ,unsigned int ScaleFrameWidth, unsigned int ScaleFrameHeight)
{        
	unsigned char CoeffType;
	int tmp = 0;

	if ( FrameWidth > 1024)
	{	
		// VFIR 2 Tap Only	
		/* video ctrl regs, scalor VFIR tap num sel = 1 */
		tmp = df_read(dfreg_video[VideoId][0]);
		tmp |= 0x10;
		df_write(dfreg_video[VideoId][0], tmp); 
	}
	else
	{
		// VFIR 4 Tap	
		/* video ctrl regs, scalor VFIR tap num sel = 0 */
		tmp = df_read(dfreg_video[VideoId][0]);
		tmp &= ~0x10;
		df_write(dfreg_video[VideoId][0], tmp); 
	}

	// VFIR
	CoeffType = (VideoId == 0) ? DF_VIDEO1_Y_VFIR_COEFF_IDX : DF_VIDEO2_Y_VFIR_COEFF_IDX;
	GenCoeff(FrameHeight, ScaleFrameHeight, CoeffType); 

	// HFIR
	CoeffType = (VideoId == 0) ? DF_VIDEO1_Y_HFIR_COEFF_IDX : DF_VIDEO2_Y_HFIR_COEFF_IDX;
	GenCoeff(FrameWidth, ScaleFrameWidth, CoeffType);

	return;
}
#endif

void DFOutIFEna(int OutIFId, int IsEna)
{
	df_outif_reg *OutIFReg= NULL;
	
	OutIFId = OutIFId & 0x1;
	OutIFReg = &dfreg.OutIF[OutIFId];
	OutIFReg->df_outif_control.val = df_read(dfreg_outif[OutIFId][0]);
	//Enable OutIF
	OutIFReg->df_outif_control.bits.iDispEna = IsEna;
	OutIFReg->df_outif_control.bits.iClkOutSel = (IsEna? 7:0);

	df_write(dfreg_outif[OutIFId][0], OutIFReg->df_outif_control.val);
	df_write(dfreg_outif[OutIFId][5],0);
	return;
}

void DFSetOutIFVideoFmt(int OutIFId, DF_VIDEO_FORMAT VideoFmt)
{
	//For Safety Format Switching It's better Disable the output clk First
	df_outif_reg *OutIFReg= NULL;

	OutIFId = OutIFId & 0x1;
	OutIFReg = &dfreg.OutIF[OutIFId];
	OutIFReg->df_outif_control.val = df_read(dfreg_outif[OutIFId][0]);
	OutIFReg->df_outif_x_size.val = df_read(dfreg_outif[OutIFId][1]);
	OutIFReg->df_outif_y_size.val = df_read(dfreg_outif[OutIFId][2]);
	OutIFReg->df_outif_active_top.val = df_read(dfreg_outif[OutIFId][3]);
	OutIFReg->df_outif_active_bot.val = df_read(dfreg_outif[OutIFId][4]);
	OutIFReg->df_outif_blank_level.val = df_read(dfreg_outif[OutIFId][5]);
	OutIFReg->df_outif_ccir_f_start.val = df_read(dfreg_outif[OutIFId][6]);
	OutIFReg->df_outif_h_sync.val = df_read(dfreg_outif[OutIFId][7]);
	OutIFReg->df_outif_v_sync_top.val = df_read(dfreg_outif[OutIFId][8]);
	OutIFReg->df_outif_v_sync_bot.val = df_read(dfreg_outif[OutIFId][9]);

	//Enable OutIF
	OutIFReg->df_outif_control.bits.iDispEna       = 1;
	OutIFReg->df_outif_control.bits.iTopOrFrameIntEna = 1;
	OutIFReg->df_outif_control.bits.iBotIntEna     = 1;
	//Set Timming
	//TV_OUTIF_TIMMING(OutIFId, VideoFmt);call tve funtion!!!!!!
	__dfOutIFTimingCfg(OutIFReg, VideoFmt);

	if(__Is_TV)
		dfreg.OutIF[OutIFId].df_outif_blank_level.val = 0x808080;
	else
		dfreg.OutIF[OutIFId].df_outif_blank_level.val = 0x202020;
//majia 2009-03-17
{
	unsigned int val = 0;

	val = video_read(0x23);
	if(OutIFId == 0){
		val &= 0xe0ffffff;
		val |= VideoFmt<<24;
	}
	else if(OutIFId == 1){
		val &= 0xffe0ffff;
		val |= VideoFmt<<16;
	}
	video_write(val, 0x23);
}

 	df_write(dfreg_outif[OutIFId][1], dfreg.OutIF[OutIFId].df_outif_x_size.val);
	df_write(dfreg_outif[OutIFId][2], dfreg.OutIF[OutIFId].df_outif_y_size.val);
	df_write(dfreg_outif[OutIFId][3], dfreg.OutIF[OutIFId].df_outif_active_top.val);
	df_write(dfreg_outif[OutIFId][4], dfreg.OutIF[OutIFId].df_outif_active_bot.val);
	df_write(dfreg_outif[OutIFId][5], dfreg.OutIF[OutIFId].df_outif_blank_level.val);
	df_write(dfreg_outif[OutIFId][6], dfreg.OutIF[OutIFId].df_outif_ccir_f_start.val);
	df_write(dfreg_outif[OutIFId][7], dfreg.OutIF[OutIFId].df_outif_h_sync.val);
	df_write(dfreg_outif[OutIFId][8], dfreg.OutIF[OutIFId].df_outif_v_sync_top.val);
	df_write(dfreg_outif[OutIFId][9], dfreg.OutIF[OutIFId].df_outif_v_sync_bot.val);
	df_write(dfreg_outif[OutIFId][0], OutIFReg->df_outif_control.val);
	return;
}

void DFSetZOrder(int OutIFId, int Gfx1ZOrder, int Gfx2ZOrder, int Video1ZOrder, int Video2ZOrder)
{
	df_compositor_reg *Reg= NULL;

	OutIFId = OutIFId & 0x1;
	Reg = &dfreg.Comp[OutIFId];
	Reg->df_comp_z_order.val = df_read(dfreg_comp[OutIFId][2]);
	
	Reg->df_comp_z_order.bits.iGfx1ZOrder   = Gfx1ZOrder;
	Reg->df_comp_z_order.bits.iGfx2ZOrder   = Gfx2ZOrder;
	Reg->df_comp_z_order.bits.iVideo1ZOrder = Video1ZOrder;
	Reg->df_comp_z_order.bits.iVideo2ZOrder = Video2ZOrder;

	df_write(dfreg_comp[OutIFId][2], Reg->df_comp_z_order.val);
	return;
}

void DFSetBackGroud(int OutIFId, unsigned char BGY, unsigned char BGU, unsigned char BGV)
{
	df_compositor_reg *Reg= NULL;

	OutIFId = OutIFId & 0x1;
	Reg = &dfreg.Comp[OutIFId];
	Reg->df_comp_back_ground.val = df_read(dfreg_comp[OutIFId][1]);
	
	Reg->df_comp_back_ground.bits.iBGY = BGY;
	Reg->df_comp_back_ground.bits.iBGU = BGU;
	Reg->df_comp_back_ground.bits.iBGV = BGV;
	
	df_write(dfreg_comp[OutIFId][1], Reg->df_comp_back_ground.val);
	return;		
}

void DFOutIFClipCfg(int OutIFId, int iClipEna, unsigned char  iClipYLow, unsigned char  iClipYRange, unsigned char  iClipCRange)
{
	OutIFId = OutIFId & 0x1;
	dfreg.Comp[OutIFId].df_comp_clip.val = df_read(dfreg_comp[OutIFId][0]);
	
	dfreg.Comp[OutIFId].df_comp_clip.bits.iClipEna    = iClipEna;
	dfreg.Comp[OutIFId].df_comp_clip.bits.iClipYLow   = iClipYLow;
	dfreg.Comp[OutIFId].df_comp_clip.bits.iClipYRange = iClipYRange;
	dfreg.Comp[OutIFId].df_comp_clip.bits.iClipCRange = iClipCRange;

	df_write(dfreg_comp[OutIFId][0], dfreg.Comp[OutIFId].df_comp_clip.val);
	return;
}

void DFVideoEna(int VideoId, int IsEna)
{
	df_video_reg *Reg= NULL;

	layer_output.vid_output[VideoId] = layer_output.vid_output[VideoId] & 0x1;
	VideoId = VideoId & 0x1;
	Reg = &dfreg.Video[VideoId];
	Reg->df_video_control_reg.val = df_read(dfreg_video[VideoId][0]);

	Reg->df_video_control_reg.bits.iVideoEna = (IsEna? (1 << layer_output.vid_output[VideoId]):0);
	DEBUG_PRINTF("layer_output.vid_output[VideoId] = %d\n",layer_output.vid_output[VideoId]);
	DEBUG_PRINTF("Reg->df_video_control_reg.bits.iVideoEna = %d\n",Reg->df_video_control_reg.bits.iVideoEna);
	df_write(dfreg_video[VideoId][0], Reg->df_video_control_reg.val);
	return;
}

void DFVideoSetLayerAlpha(int VideoId, unsigned char Alpha, unsigned char KeyAlpha0, unsigned char KeyAlpha1)
{
	df_video_reg *Reg= NULL;

	VideoId = VideoId & 0x1;
	Reg = &dfreg.Video[VideoId];
	Reg->df_video_alpha_control_reg.val = df_read(dfreg_video[VideoId][1]);
	
	Reg->df_video_alpha_control_reg.bits.iDefaultAlpha = Alpha & 0xFF;
	Reg->df_video_alpha_control_reg.bits.iLumaKeyAlpha0 = KeyAlpha0;
	Reg->df_video_alpha_control_reg.bits.iLumaKeyAlpha1 = KeyAlpha1;

	df_write(dfreg_video[VideoId][1], Reg->df_video_alpha_control_reg.val);
	return;
}

void DFVideoSetLumaKey(int VideoId, int IsKeyEna, unsigned char LumaKeyMin, unsigned char LumaKeyMax)
{
	df_video_reg *Reg= NULL;

	VideoId = VideoId & 0x1;
	Reg = &dfreg.Video[VideoId];
	Reg->df_video_luma_key_reg.val = df_read(dfreg_video[VideoId][2]);
	Reg->df_video_control_reg.val = df_read(dfreg_video[VideoId][0]);
	
	if(IsKeyEna)
	{
		Reg->df_video_control_reg.bits.iLumaKeyEna = 1;
		Reg->df_video_luma_key_reg.bits.iLumaKeyMin = LumaKeyMin;
		Reg->df_video_luma_key_reg.bits.iLumaKeyMax = LumaKeyMax;
	}
	else
	{
		Reg->df_video_control_reg.bits.iLumaKeyEna = 0;
	}

	df_write(dfreg_video[VideoId][2], Reg->df_video_luma_key_reg.val);
	df_write(dfreg_video[VideoId][0], Reg->df_video_control_reg.val);
	return;	
}

//Make Suret the Des Windows Is the Same
void DFVideoSetSrcCrop(int VideoId, df_rect CropRect)
{
	df_video_reg *Reg= NULL;

	VideoId = VideoId & 0x1;
	Reg = &dfreg.Video[VideoId];
	Reg->df_video_src_x_crop_reg.val = df_read(dfreg_video[VideoId][5]);
	Reg->df_video_src_y_crop_reg.val = df_read(dfreg_video[VideoId][6]);
	
	Reg->df_video_src_x_crop_reg.bits.iSrcCropXOff    = CropRect.left & (~0xF);
	Reg->df_video_src_x_crop_reg.bits.iSrcCropXWidth  = CropRect.right & (~0xF);
	Reg->df_video_src_y_crop_reg.bits.iSrcCropYOff    = CropRect.top & (~0x3);  
	Reg->df_video_src_y_crop_reg.bits.iSrcCropYHeight = CropRect.bottom & (~0x3);

	df_write(dfreg_video[VideoId][5], Reg->df_video_src_x_crop_reg.val);
	df_write(dfreg_video[VideoId][6], Reg->df_video_src_y_crop_reg.val);
	return;
}

//Make Suret the Des Windows Is the Same, Without DispX Crop
void DFVideoSetDispWin(int VideoId, df_rect DispRect)
{
	df_video_reg *Reg= NULL;

	VideoId = VideoId & 0x1;
	Reg = &dfreg.Video[VideoId];

	DispRect.top &= (~0x3);
	DispRect.bottom &= (~0x3);
	Reg->df_video_x_position_reg.val = df_read(dfreg_video[VideoId][3]);
	Reg->df_video_y_position_reg.val = df_read(dfreg_video[VideoId][4]);

	Reg->df_video_x_position_reg.bits.iDispXCropEnable = 0;
	Reg->df_video_x_position_reg.bits.iDispXStart = DispRect.left;
	Reg->df_video_x_position_reg.bits.iDispXEnd   = DispRect.right;
	Reg->df_video_y_position_reg.bits.iDispYStart = DispRect.top & (~0x3);
	Reg->df_video_y_position_reg.bits.iDispYEnd   = DispRect.bottom & (~0x3);
	//Any Error Check with Screen Size?

	df_write(dfreg_video[VideoId][3], Reg->df_video_x_position_reg.val);
	df_write(dfreg_video[VideoId][4], Reg->df_video_y_position_reg.val);

	Reg->df_video_x_position_reg.val = df_read(dfreg_video[VideoId][3]);
	Reg->df_video_y_position_reg.val = df_read(dfreg_video[VideoId][4]);

	return;
}

int DFVideoIsDispWinChange(int VideoId, df_rect DispRect, df_rect SrcRect)
{
	df_video_reg *Reg= NULL;
	int retval = 0;

	VideoId = VideoId & 0x1;
	Reg = &dfreg.Video[VideoId];

	DispRect.top &= (~0x3);
	DispRect.bottom &= (~0x3);
	DispRect.left &= (~0xf);
	DispRect.right &= (~0xf);
	SrcRect.top &= (~0x3);
	SrcRect.bottom &= (~0x3);
	SrcRect.left &= (~0xf);
	SrcRect.right &= (~0xf);

	Reg->df_video_x_position_reg.val = df_read(dfreg_video[VideoId][3]);
	Reg->df_video_y_position_reg.val = df_read(dfreg_video[VideoId][4]);
	Reg->df_video_src_x_crop_reg.val = df_read(dfreg_video[VideoId][5]);
	Reg->df_video_src_y_crop_reg.val = df_read(dfreg_video[VideoId][6]);

	if((Reg->df_video_x_position_reg.bits.iDispXStart == DispRect.left)&&(Reg->df_video_x_position_reg.bits.iDispXEnd == DispRect.right)&&
		(Reg->df_video_y_position_reg.bits.iDispYStart == DispRect.top)&&(Reg->df_video_y_position_reg.bits.iDispYEnd == DispRect.bottom)){
		if((Reg->df_video_src_x_crop_reg.bits.iSrcCropXOff == SrcRect.left)&&(Reg->df_video_src_x_crop_reg.bits.iSrcCropXWidth == SrcRect.right)&&
		(Reg->df_video_src_y_crop_reg.bits.iSrcCropYOff == SrcRect.top)&&(Reg->df_video_src_y_crop_reg.bits.iSrcCropYHeight == SrcRect.bottom)){
			return 1;
		}
		else{
			return 2;
		}
	}
	else {
		return 0;
	}

	return 0;
}

//Make Sure the Des Windows Is the Same, With DispX Crop
void DFVideoSetDispWinWithXCrop(int VideoId, df_rect DispRect, unsigned int DispXStartCrop, unsigned int DispXEndCrop)
{
	df_video_reg *Reg= NULL;

	VideoId = VideoId & 0x1;
	Reg = &dfreg.Video[VideoId];

	//DispXOffset &= (~0xF);
	//DispXWidth  &= (~0xF);
	DispRect.top &= (~0x3);
	DispRect.bottom &= (~0x3);
	Reg->df_video_x_position_reg.val = df_read(dfreg_video[VideoId][3]);
	Reg->df_video_y_position_reg.val = df_read(dfreg_video[VideoId][4]);
	
	Reg->df_video_x_position_reg.bits.iDispXCropEnable   = 1;
	Reg->df_video_x_position_reg.bits.iDispXStartCropPixelNum = DispXStartCrop;
	Reg->df_video_x_position_reg.bits.iDispXEndCropPixelNum  = DispXEndCrop;
	Reg->df_video_x_position_reg.bits.iDispXStart = DispRect.left;
	Reg->df_video_x_position_reg.bits.iDispXEnd   = DispRect.left + DispRect.right;
	Reg->df_video_y_position_reg.bits.iDispYStart = DispRect.top;
	Reg->df_video_y_position_reg.bits.iDispYEnd   = DispRect.top + DispRect.bottom;
	//Any Error Check with Screen Size?

	df_write(dfreg_video[VideoId][3], Reg->df_video_x_position_reg.val);
	df_write(dfreg_video[VideoId][4], Reg->df_video_y_position_reg.val);
	return;
}

void DFVideoEnaColorModulator(int VideoId, int IsEnable)
{
	df_video_reg *Reg= NULL;

	VideoId = VideoId & 0x1;
	Reg = &dfreg.Video[VideoId];

	Reg->df_video_control_reg.val = df_read(dfreg_video[VideoId][0]);
	Reg->df_video_control_reg.bits.iColorModulatorEna = IsEnable;
	df_write(dfreg_video[VideoId][0], Reg->df_video_control_reg.val);
	return;
}

enum _YUV2RGB_COEFF_IDX_
{
	YUV_SD_TO_RGB = 0,
	YUV_HD_TO_RGB = 1,
	YUV_SD_TO_YUV_HD = 2,
	YUV_HD_TO_YUV_SD = 3,
	YUV2RGB_COEFF_NUM,
};

static int ColorModulatorCoeff[YUV2RGB_COEFF_NUM][12] = 
{
	{
		74,		0,		102,		4319,
		74,		281,		308,		135,
		74,		129,		0,		4373,
	},
	{
		74,		0,		115,		4344,
		74,		270,		290,		77,
		74,		135,		0,		4385,
	},
	{
		64,		263,		269,		41,
		0,		65,		7,		4113,
		0,		5,		66,		4109,
	},
	{
		64,		6,		12,		4133,
		0,		63,		263,		15,
		0,		261,		63,		11,
	},
};

void DFVideoSetColorModulatorCoeff(int VideoId, unsigned int cm_index)
{
	df_video_reg *Reg= NULL;

	VideoId = VideoId & 0x1;
	Reg = &dfreg.Video[VideoId];

	Reg->df_video_cm_coeff0_012.val = df_read(dfreg_video[VideoId][7]);
	Reg->df_video_cm_coeff0_3.val = df_read(dfreg_video[VideoId][8]);
	Reg->df_video_cm_coeff0_012.val = df_read(dfreg_video[VideoId][9]);
	Reg->df_video_cm_coeff0_3.val = df_read(dfreg_video[VideoId][10]);
	Reg->df_video_cm_coeff0_012.val = df_read(dfreg_video[VideoId][11]);
	Reg->df_video_cm_coeff0_3.val = df_read(dfreg_video[VideoId][12]);

	Reg->df_video_cm_coeff0_012.bits.coeff0 = ColorModulatorCoeff[cm_index][0];
	Reg->df_video_cm_coeff0_012.bits.coeff1 = ColorModulatorCoeff[cm_index][1];
	Reg->df_video_cm_coeff0_012.bits.coeff2 = ColorModulatorCoeff[cm_index][2];
	Reg->df_video_cm_coeff0_3.bits.coeff3 = ColorModulatorCoeff[cm_index][3];
	Reg->df_video_cm_coeff1_012.bits.coeff0 = ColorModulatorCoeff[cm_index][4];
	Reg->df_video_cm_coeff1_012.bits.coeff1 = ColorModulatorCoeff[cm_index][5];
	Reg->df_video_cm_coeff1_012.bits.coeff2 = ColorModulatorCoeff[cm_index][6];
	Reg->df_video_cm_coeff1_3.bits.coeff3 = ColorModulatorCoeff[cm_index][7];
	Reg->df_video_cm_coeff2_012.bits.coeff0 = ColorModulatorCoeff[cm_index][8];
	Reg->df_video_cm_coeff2_012.bits.coeff1 = ColorModulatorCoeff[cm_index][9];
	Reg->df_video_cm_coeff2_012.bits.coeff2 = ColorModulatorCoeff[cm_index][10];
	Reg->df_video_cm_coeff2_3.bits.coeff3 = ColorModulatorCoeff[cm_index][11];

	df_write(dfreg_video[VideoId][7], Reg->df_video_cm_coeff0_012.val);
	df_write(dfreg_video[VideoId][8], Reg->df_video_cm_coeff0_3.val);
	df_write(dfreg_video[VideoId][9], Reg->df_video_cm_coeff1_012.val);
	df_write(dfreg_video[VideoId][10], Reg->df_video_cm_coeff1_3.val);
	df_write(dfreg_video[VideoId][11], Reg->df_video_cm_coeff2_012.val);
	df_write(dfreg_video[VideoId][12], Reg->df_video_cm_coeff2_3.val);

	return;	
}

void DFGfxEna(int GfxId, int IsEna, int IsRGB2YUVEna)
{
	df_gfx_reg *Reg= NULL;

	layer_output.gfx_output[GfxId] = layer_output.gfx_output[GfxId] & 0x1;
	GfxId   = GfxId & 0x1;
	Reg = &dfreg.Gfx[GfxId];

	Reg->df_gfx_control_reg.val = df_read(dfreg_gfx[GfxId][0]);

	if(IsEna)
	{
		Reg->df_gfx_control_reg.bits.iGfxEnable = (1 << layer_output.gfx_output[GfxId]);
		Reg->df_gfx_control_reg.bits.iRGB2YUVConvertEna = IsRGB2YUVEna;
	}
	else
	{
		Reg->df_gfx_control_reg.bits.iGfxEnable = 0;
	}

	df_write(dfreg_gfx[GfxId][0], Reg->df_gfx_control_reg.val);

	return;
}

void DFGfxSetAlpha(int GfxId, unsigned char DefaultAlpha, unsigned char ARGB1555Alpha0, unsigned char ARGB1555Alpha1)
{
	df_gfx_reg *Reg= NULL;

	GfxId   = GfxId & 0x1;
	Reg = &dfreg.Gfx[GfxId];

	Reg->df_gfx_alpha_control_reg.val = df_read(dfreg_gfx[GfxId][2]);

	Reg->df_gfx_alpha_control_reg.bits.iDefaultAlpha   = DefaultAlpha;
	Reg->df_gfx_alpha_control_reg.bits.iArgb1555Alpha0 = ARGB1555Alpha0;
	Reg->df_gfx_alpha_control_reg.bits.iArgb1555Alpha1 = ARGB1555Alpha1;

	df_write(dfreg_gfx[GfxId][2], Reg->df_gfx_alpha_control_reg.val);
	return;
}

void DFGfxSetColorKey(int GfxId, unsigned char KeyRMin, unsigned char KeyRMax, unsigned char KeyGMin, unsigned char KeyGMax, unsigned char KeyBMin, unsigned char KeyBMax)
{
	df_gfx_reg *Reg= NULL;
	
	GfxId   = GfxId & 0x1;
	Reg = &dfreg.Gfx[GfxId];

	Reg->df_gfx_key_red_reg.val = df_read(dfreg_gfx[GfxId][3]);
	Reg->df_gfx_key_green_reg.val = df_read(dfreg_gfx[GfxId][5]);
	Reg->df_gfx_key_blue_reg.val = df_read(dfreg_gfx[GfxId][4]);
	
	Reg->df_gfx_key_red_reg.bits.iKeyRedMin   = KeyRMin;
	Reg->df_gfx_key_red_reg.bits.iKeyRedMax   = KeyRMax;
	Reg->df_gfx_key_green_reg.bits.iKeyGreenMin = KeyGMin;
	Reg->df_gfx_key_green_reg.bits.iKeyGreenMax = KeyGMax;
	Reg->df_gfx_key_blue_reg.bits.iKeyBlueMin  = KeyBMin;
	Reg->df_gfx_key_blue_reg.bits.iKeyBlueMax  = KeyBMax;

	df_write(dfreg_gfx[GfxId][3], Reg->df_gfx_key_red_reg.val);
	df_write(dfreg_gfx[GfxId][5], Reg->df_gfx_key_green_reg.val);
	df_write(dfreg_gfx[GfxId][4], Reg->df_gfx_key_blue_reg.val);

	return;
}

void DFGfxColorKeyEna(int GfxId,int IsEna)
{
	df_gfx_reg*Reg = NULL;
	
	GfxId = GfxId & 0x1;
	Reg = &dfreg.Gfx[GfxId];
	Reg->df_gfx_control_reg.val = df_read(dfreg_gfx[GfxId][0]);
	
	Reg->df_gfx_control_reg.bits.iColorKeyEnable = IsEna;

	df_write(dfreg_gfx[GfxId][0], Reg->df_gfx_control_reg.val);

	return;
}

int DFSetGfxClutTable(int GfxId, unsigned char *ClutTable, int ClutDataLen)
{
	int i = 0;
	df_gfx_reg *Reg = &dfreg.Gfx[GfxId & 0x1];

	//Valid ClutTable Update Time is Gfx Off or Disp Blank Region
	__df_update_start();
	for (i = 0; i < ClutDataLen; i++)
	{
		Reg->df_gfx_clut_addr_reg.bits.iClutAddr = i;
		Reg->df_gfx_clut_data_reg.val = ClutTable[i];
		df_write(dfreg_gfx[GfxId][11], Reg->df_gfx_clut_addr_reg.val);
		df_write(dfreg_gfx[GfxId][12], Reg->df_gfx_clut_data_reg.val);
	}
	__df_update_end();
}

//#define BY_PASS_SCA  1
//#define IS_FRAME   0
//#define IS_HD    0
#define VPhase    0
#define HPhase    0
#define Buffer_Depth  3	// 4

unsigned int Capture_width = 720;
unsigned int Capture_height  = 576;
unsigned int Scaler_width = 720;

#define BUF_PITCH   (1024*704)
#define Y_Top_Addr   (HD2SD_DATA_REGION)
#define C_Top_Addr   (HD2SD_DATA_REGION + (BUF_PITCH * Buffer_Depth))

unsigned int by_pass_sca = 0;
unsigned int is_frame = 0;
unsigned int is_hd = 0;
void DFHD2SDCaptureEna(int IsEnable, int OutIFId)
{
	df_hd2sd_reg *Reg= NULL;

	Reg = &dfreg.HD2SD;
	Reg->df_hd2sd_control.val = df_read(dfreg_hd2sd[0]);
	Reg->df_hd2sd_control.bits.iHD2SDEna = IsEnable;
	Reg->df_hd2sd_control.bits.iCompositorSel = OutIFId;
	df_write(dfreg_hd2sd[0], Reg->df_hd2sd_control.val);
}

#if defined(DEFAULT_SCALER)
void __DFHD2SDScalerCfg(void)
{
	int reg = 0;
	int CoeffType = 4;
	int phaseindex, tapindex;

	// HFIR
	for (phaseindex = 0; phaseindex < DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM; phaseindex++)
	{ 
		for(tapindex = 0; tapindex < DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM; tapindex++)
		{
			int Coeff = YHFIRCoeff[phaseindex][tapindex];

			reg = ((tapindex) <<  0) | ((phaseindex) <<  8) | ((CoeffType) << 16) ;
			df_write(DISP_SCA_COEF_IDX, reg); 
			df_write(DISP_SCA_COEF_DATA, Coeff);
		}  
	}

	return;
}
#else
void DFSetHD2SDScaler(unsigned int FrameWidth, unsigned int ScaleFrameWidth)
{	
	unsigned int reg = 0;
	int CoeffType = 4;

	reg = df_read(DISP_HD2SD_CTRL);	
	if ((reg >> 2) & 0x1)
		return;	

	GenCoeff(FrameWidth, ScaleFrameWidth, CoeffType);
}
#endif
void DFHD2SDConfig(void)
{
	unsigned int reg = 0;

	df_hd2sd_reg *Reg= NULL;

	Reg = &dfreg.HD2SD;
	Reg->df_hd2sd_control.val = df_read(dfreg_hd2sd[0]);
	Reg->df_hd2sd_control.bits.iByPassScaler = by_pass_sca;
	Reg->df_hd2sd_control.bits.iIsFrame = is_frame;
	Reg->df_hd2sd_control.bits.iIsHD = is_hd;
	Reg->df_hd2sd_control.bits.iDramFIFODepthMinus1 = Buffer_Depth - 1;
	Reg->df_hd2sd_control.bits.iVInitPhase = VPhase;
	Reg->df_hd2sd_control.bits.iHInitPhase = HPhase;
	df_write(dfreg_hd2sd[0], Reg->df_hd2sd_control.val);

	if (is_frame)
		reg = Capture_width | (Capture_height << 16);
	else
		reg = Capture_width | ((Capture_height >> 1) << 16);

	df_write(DISP_HD2SD_DES_SIZE, reg);

	reg = Y_Top_Addr >> 3;
	df_write(DISP_HD2SD_ADDR_Y, reg);

	reg = C_Top_Addr >> 3;
	df_write(DISP_HD2SD_ADDR_C, reg);

	reg = BUF_PITCH >> 3;
	df_write(DISP_HD2SD_BUF_PITCH, reg);

#if defined(DEFAULT_SCALER)
	__DFHD2SDScalerCfg();
#else
	DFSetHD2SDScaler(Scaler_width, Capture_width);
#endif
	return;
}

//1REG MAP
int __df_mem_initialize(void)
{
	if (NULL == request_mem_region(DISP_REG_BASE, DISP_REG_SIZE, "Orion15 DF space")) {
		goto INIT_ERR0;
	}

	if (NULL == disp_base){
		if(!(disp_base = (unsigned char *)ioremap(DISP_REG_BASE, DISP_REG_SIZE))) {
			goto INIT_ERR1;
		}
	}

	if (NULL == request_mem_region(TVE0_REG_BASE, TVE0_REG_SIZE, "Orion15 TVEspace")) {
		goto INIT_ERR2;
	}

	if (NULL == tve0_base){
		if(!(tve0_base = (unsigned char *)ioremap(TVE0_REG_BASE, TVE0_REG_SIZE))) {
			goto INIT_ERR3;
		}
	}

	if (NULL == request_mem_region(TVE1_REG_BASE, TVE1_REG_SIZE, "Orion15 TVEspace")) {
		goto INIT_ERR4;
	}

	if (NULL == tve1_base){
		if(!(tve1_base = (unsigned char *)ioremap(TVE1_REG_BASE, TVE1_REG_SIZE))) {
			goto INIT_ERR5;
		}
	}

	if (NULL == clk_base){
		if(!(clk_base = (unsigned char *)ioremap(REG_CLK_BASE, REG_CLK_SIZE))){
			goto INIT_ERR6;
		}
	}

	if (NULL == ttx_ctrl){
		if(!(ttx_ctrl = (unsigned char *)ioremap(TTX_ENGINE_BASE, 0x200))){
			goto INIT_ERR7;
		}
	}

	if (NULL == wss_base){
		if(!(wss_base = (unsigned char *)ioremap(ORION_WSS_BASE , ORION_WSS_SIZE))) {
			goto INIT_ERR8;
		}
	}

	DEBUG_PRINTF("disp_base 0x%x, tve0_base 0x%x, tve1_base 0x%x, clk_base 0x%x\n",(unsigned int)disp_base,(unsigned int)tve0_base,(unsigned int)tve1_base,(unsigned int)clk_base);
	return 0;

	INIT_ERR8:
	printk(KERN_INFO "__df_mem_initialize INIT_ERR8 ...\n");
	iounmap((void *)ttx_ctrl);
	INIT_ERR7:
	printk(KERN_INFO "__df_mem_initialize INIT_ERR7 ...\n");
	iounmap((void *)clk_base);
	INIT_ERR6:
	printk(KERN_INFO "__df_mem_initialize INIT_ERR6 ...\n");
	iounmap((void *)tve1_base);
	INIT_ERR5:
	printk(KERN_INFO "__df_mem_initialize INIT_ERR5 ...\n");
	release_mem_region(TVE1_REG_BASE, TVE1_REG_SIZE);
	INIT_ERR4:
	printk(KERN_INFO "__df_mem_initialize INIT_ERR4 ...\n");
	iounmap((void *)tve0_base);
	INIT_ERR3:
	printk(KERN_INFO "__df_mem_initialize INIT_ERR3 ...\n");
	release_mem_region(TVE0_REG_BASE, TVE0_REG_SIZE);
	INIT_ERR2:
	printk(KERN_INFO "__df_mem_initialize INIT_ERR2 ...\n");
	iounmap((void *)disp_base);
	INIT_ERR1:
	printk(KERN_INFO "__df_mem_initialize INIT_ERR1 ...\n");
	release_mem_region(DISP_REG_BASE, DISP_REG_SIZE);
	INIT_ERR0:
	printk(KERN_INFO "__df_mem_initialize INIT_ERR0 ...\n");
	return -1;
}

void __df_mem_destroy(void)
{
	dma_pool_free(pool, (void*)ttx_buf, dma_phy_addr);  
	dma_pool_destroy(pool);

	iounmap((void *)wss_base);
	iounmap((void *)ttx_ctrl);

	iounmap((void *)clk_base);
	iounmap((void *)tve1_base);
	release_mem_region(TVE1_REG_BASE, TVE1_REG_SIZE);
	iounmap((void *)tve0_base);
	release_mem_region(TVE0_REG_BASE, TVE0_REG_SIZE);
	iounmap((void *)disp_base);
	release_mem_region(DISP_REG_BASE, DISP_REG_SIZE);
}

void __df_calc_video_position(TVOUT_MODE pre_mode,TVOUT_MODE cur_mode,struct df_output_pos *pos)
{
	switch(cur_mode){
		case TVOUT_MODE_576I:
		case TVOUT_MODE_576P:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 576;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 720;
			break;
		case TVOUT_MODE_480I:
		case TVOUT_MODE_480P:
		case TVOUT_MODE_PAL_M:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 480;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 720;
			break;
		case TVOUT_MODE_720P50:
		case TVOUT_MODE_720P60:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 720;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 1280;
			break;
		case TVOUT_MODE_1080I25:
		case TVOUT_MODE_1080I30:
		case TVOUT_MODE_1080P24:
		case TVOUT_MODE_1080P25:
		case TVOUT_MODE_1080P30:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 1080;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 1920;
			break;
		case TVOUT_RGB_640X480_60FPS:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 480;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 640;
			break;
		case TVOUT_RGB_800X600_60FPS:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 600;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 800;
			break;
		case TVOUT_RGB_800X600_72FPS:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 600;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 800;
			break;
		case TVOUT_RGB_1024X768_60FPS:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 768;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 1024;
			break;
		case TVOUT_RGB_1280X1024_50FPS:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 1024;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 1280;
			break;
		case TVOUT_RGB_1280X1024_60FPS:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 1024;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 1280;
			break;
		case TVOUT_RGB_1600X1000_60FPS:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 1000;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 1600;
			break;
		case TVOUT_RGB_1280X720_60FPS:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 720;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 1280;
			break;
		case TVOUT_RGB_848X480_60FPS:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 848;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 480;
			break;
		case TVOUT_RGB_800X480_60FPS:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 800;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 480;
			break;
		default:
			pos->src.top = pos->dst.top = 0;
			pos->src.bottom = pos->dst.bottom = 576;
			pos->src.left = pos->dst.left = 0;
			pos->src.right = pos->dst.right = 720;
			break;
	}
}

/*=============================== CONFIG_WSS =====================================*/

//static u32 DevBaseAddr = TVE_BASE_ADDR;
#define DevBaseAddr		(tve1_base)


#if 0
/*-------------------------------------------------------------------------
 * Function : TVE_SetVbiConfig
 *	      Set the parameters
 * Input    : 
 * Output   :
 * Return   : Error Code
 * ----------------------------------------------------------------------*/
int WSS_Ctrl(u8 Enable)
{
	int	   Error = 0;

	u8 ValCfg = 0;

	//ValCfg = wss_readb(TVE_WSS_CONFIG);
	printk(KERN_INFO "====ValCfg=0x%x\n",ValCfg);
	ValCfg=wss_base[TVE_WSS_CONFIG];
	printk(KERN_INFO "111====ValCfg=0x%x\n",ValCfg);
	if(Enable == 1) {
		ValCfg |= 0x3;
	} else {
		ValCfg &= ~0x3;
	}

	TVE_WriteRegDev8(DevBaseAddr, TVE_WSS_CONFIG, ValCfg);
	printk(KERN_INFO "=222===ValCfg=0x%x\n",ValCfg);
	ValCfg=0;
	//wss_base[TVE_WSS_CONFIG]=ValCfg;

	ValCfg=wss_base[TVE_WSS_CONFIG];
	printk(KERN_INFO "=333===ValCfg=0x%x\n",ValCfg);

	return ( Error );
}

/*-------------------------------------------------------------------------
 * Function : TVE_SetVbiConfig
 *	      Set the parameters
 * Input    : 
 * Output   :
 * Return   : Error Code
 * ----------------------------------------------------------------------*/
int WSS_SetConfig(VBI_WssStandard_t WssStandard)
{
	int	   Error = 0;

	u8 ValCfg = 0;
	u16 ValClk = 0;
	u16 ValLevel = 0;
	u8 LineNum = 0;
	u8 Hichar=0,Lochar=0;


	if(WssStandard == VBI_WSS_PAL) {		
		ValCfg = 0x64;

		ValClk = 0x17b;
		ValLevel = 642;
		LineNum = 22;
	} else {
		ValCfg = 0x50;

		ValClk = 0x10f;
		ValLevel = 632;	
		LineNum = 19;
	}

	TVE_WriteRegDev8(DevBaseAddr, TVE_WSS_CONFIG, ValCfg);
	//wss_base[TVE_WSS_CONFIG]=ValCfg;

	TVE_WriteRegDev12(DevBaseAddr, TVE_WSS_CLOCK_BASE, ValClk);
	//Lochar=ValClk;
	//Hichar=(ValClk>>8)&0x0F;
	//wss_base[TVE_WSS_CLOCK_BASE]=Lochar;
	//wss_base[TVE_WSS_CLOCK_BASE+1]=wss_base[TVE_WSS_CLOCK_BASE+1]|Hichar;
	TVE_WriteRegDev10(DevBaseAddr, TVE_WSS_LEVEL_BASE, ValLevel);
	//Lochar=ValLevel;
	//Hichar=(ValLevel>>8)&0x03;
	//wss_base[TVE_WSS_LEVEL_BASE]=Lochar;
	//wss_base[TVE_WSS_LEVEL_BASE+1]=wss_base[TVE_WSS_LEVEL_BASE+1]|Hichar;

	TVE_WriteRegDev8(DevBaseAddr, TVE_WSS_LINEF0, LineNum);
	//wss_base[TVE_WSS_LINEF0]=LineNum;
	TVE_WriteRegDev8(DevBaseAddr, TVE_WSS_LINEF1, LineNum);
	//wss_base[TVE_WSS_LINEF1]=LineNum;

	return ( Error );
}

/************************************************************************************************************
Name        : WriteVbiData
Description : get
Parameters  : 
Assumptions :
Limitations :
Returns     : CS_ErrorCode_t type value :

 *************************************************************************************************************/

static int WSS_WriteData( WSS_VbiData_t *const VbiData_p)
{
	int Error = 0;
	u8 Hichar=0,Lochar=0;

	TVE_VsyncType_t FieldType;

	FieldType = VbiData_p->FieldType;

	switch(VbiData_p->WssType) {
		case TVE_VBI_WSS:
			{
				u16 *Data_p;
				Data_p = (u16 *)VbiData_p->VbiData.Buf;

				if(FieldType == TVE_FIELD_TOP) {
					TVE_WriteRegDev16BE(DevBaseAddr, TVE_WSS_DATAF0_BASE+1, *Data_p);	
					//Lochar=*Data_p;
					//Hichar=((*Data_p)>>8);
					//wss_base[TVE_WSS_DATAF0_BASE+1]=Lochar;
					//wss_base[TVE_WSS_DATAF0_BASE+2]=Hichar;
				} else {
					TVE_WriteRegDev16BE(DevBaseAddr, TVE_WSS_DATAF1_BASE+1, *Data_p);	
					//Lochar=*Data_p;
					//Hichar=((*Data_p)>>8);
					//wss_base[TVE_WSS_DATAF1_BASE+1]=Lochar;
					//wss_base[TVE_WSS_DATAF1_BASE+2]=Hichar;
				}
			}
			break;			

		case TVE_VBI_CGMS:
			{
				u32 *Data_p;
				Data_p = (u32 *)VbiData_p->VbiData.Buf;

				if(FieldType == TVE_FIELD_TOP) {
					TVE_WriteRegDev20BE(DevBaseAddr, TVE_WSS_DATAF0_BASE, *Data_p);
					//Lochar=*Data_p;
					//Hichar=((*Data_p)>>8);
					//wss_base[TVE_WSS_DATAF0_BASE]=Lochar;
					//wss_base[TVE_WSS_DATAF0_BASE+1]=Hichar;
				} else {
					TVE_WriteRegDev20BE(DevBaseAddr, TVE_WSS_DATAF1_BASE, *Data_p);	
					//Lochar=*Data_p;
					//Hichar=((*Data_p)>>8);
					//wss_base[TVE_WSS_DATAF1_BASE]=Lochar;
					//wss_base[TVE_WSS_DATAF1_BASE+1]=Hichar;
				}
			}
			break;	

		default:
			break;
	}

	return (Error);
}

/*-------------------------------------------------------------------------
 * Function : TTVO_WriteCcData
 * Input    : 
 * Output   :
 * Return   : TRUE if error, FALSE if success
 * ----------------------------------------------------------------------*/
int VBI_SetWssInfo( VBI_WssInfo_t *WssInfo_p)
{
	int error_code = 0;

	u8 ValAr;
	u8 DataBuf[3];

	WSS_VbiData_t VbiData;

	DataBuf[1] = 0;

	switch(WssInfo_p->ARatio) {
		case WSS_AR_4TO3_FULL:
			ValAr= 0x08;
			break;	

		case WSS_AR_14TO9_BOX_CENTER:
			ValAr= 0x01;
			break;	

		case WSS_AR_14TO9_BOX_TOP:
			ValAr= 0x02;
			break;	

		case WSS_AR_16TO9_BOX_CENTER:
			ValAr= 0x0b;
			break;	

		case WSS_AR_16TO9_BOX_TOP:
			ValAr= 0x04;
			break;	

		case WSS_AR_16TO9P_BOX_CENTER:
			ValAr= 0x0d;
			break;	

		case WSS_AR_14TO9_FULL:
			ValAr= 0x0e;
			break;	

		case WSS_AR_16TO9_FULL:
			ValAr= 0x07;
			break;	

		default:
			ValAr= 0x08;	/* ehnus Default: WSS_AR_4TO3_FULL */
			break;
	}

	DataBuf[0] = ValAr;

	VbiData.WssType = WssInfo_p->WssType;

	VbiData.FieldType = TVE_FIELD_TOP;	
	VbiData.VbiData.Buf = DataBuf;
	error_code = WSS_WriteData( &VbiData);
	if(error_code != 0) {
		printk(" error VOS_WriteVbiData\n");
	}	

	VbiData.FieldType = TVE_FIELD_BOT;	
	VbiData.VbiData.Buf = DataBuf;
	error_code = WSS_WriteData( &VbiData);
	if(error_code != 0) {
		printk(" error VOS_WriteVbiData\n");
	}	

	return error_code;

}
#endif

void ttx_dump_data(unsigned char *tbuf)
{
	int i, j;
	unsigned char *ptr = tbuf;
	printk("ttx_packets_dump:\n");
	for (i = 0; i < 16; i++) {
		printk("--------------------------------- %d ---------------------------------\n", i);
		for (j = 0; j < 42; j++, ptr++) {
			printk("0x%02x ", *ptr);
			if ((0 == (j % 14)) && !j)
				printk("\n");
		}
		printk("\n");
	}
}

/*-------------------------------------------------------------------------
 * Function : ttx_control
 *	      Set the parameters
 * Input    : 
 * Output   :
 * Return   : None
 * ----------------------------------------------------------------------*/
void ttx_control(unsigned int enable)
{
	unsigned char ttx_conf;

	/* disable : mips of video */
	TVE_WriteRegDev32(ttx_ctrl, TTX_ENG_ENABLE, 0);

	ttx_conf = TVE_ReadRegDev8(DevBaseAddr, TVE_TTX_CONFIG);
	if(enable) {
		ttx_conf |= 0x1;
		TVE_WriteRegDev8(DevBaseAddr, TVE_TTC_ENA, 1);
	} else {
		ttx_conf &= ~0x1;
		TVE_WriteRegDev8(DevBaseAddr, TVE_TTC_ENA, 0);
	}

	/* ttx encoder */
	TVE_WriteRegDev8(DevBaseAddr, TVE_TTX_CONFIG, ttx_conf);
}

/*-------------------------------------------------------------------------
 * Function : ttx_setconfig
 *	      Set the parameters
 * Input    : 
 * Output   :
 * Return   : None
 * ----------------------------------------------------------------------*/
void ttx_setconfig(VBI_TxtStandard_t TxtStandard)
{
	struct {
		unsigned char ValCfg;

		unsigned short ValClk;
		unsigned short ValLevel;
		unsigned char LineNum0;
		unsigned char LineNum1;

		unsigned char HStart;
		unsigned char HLength;
	} ttx_config[2] = {
		{ 0x1c, 0x41c, 613, 6, 21, 72, 42 }, /* VBI_TXT_PALB */
		{ 0x1c, 0x365, 635, 9, 20, 55, 34 }  /* VBI_TXT_NTSCB */
	};

	TVE_WriteRegDev8(DevBaseAddr, TVE_TTX_CONFIG, ttx_config[TxtStandard].ValCfg);

	TVE_WriteRegDev8(DevBaseAddr, TVE_TTX_STARTPOS, ttx_config[TxtStandard].HStart);
	TVE_WriteRegDev8(DevBaseAddr, TVE_TTX_DATALENGTH, ttx_config[TxtStandard].HLength);
	TVE_WriteRegDev8(DevBaseAddr, TVE_TTX_FCODE, 0x27);

	TVE_WriteRegDev12(DevBaseAddr, TVE_TTX_CLOCK_BASE, ttx_config[TxtStandard].ValClk);
	TVE_WriteRegDev10(DevBaseAddr, TVE_TTX_LEVEL_BASE, ttx_config[TxtStandard].ValLevel);
	TVE_WriteRegDev10(DevBaseAddr, TVE_TTX_LINEF0START_BASE, ttx_config[TxtStandard].LineNum0);
	TVE_WriteRegDev10(DevBaseAddr, TVE_TTX_LINEF1START_BASE, ttx_config[TxtStandard].LineNum0);
	TVE_WriteRegDev10(DevBaseAddr, TVE_TTX_LINEF0END_BASE, ttx_config[TxtStandard].LineNum1);	
	TVE_WriteRegDev10(DevBaseAddr, TVE_TTX_LINEF1END_BASE, ttx_config[TxtStandard].LineNum1);
}

#if 0
/*-------------------------------------------------------------------------
 * Function : VOUT_SetCompChan
 *	      Set the parameters
 * Input    : 
 * Output   :
 * Return   : Error Code
 * ----------------------------------------------------------------------*/
int VOUT_SetCompChan( CSVOUT_CompChannType_t CompChan)
{
	int Error = 0;
	u8 ValOutputCtrl= 0;

	//ValOutputCtrl = wss_readb(TVE_OUTPUT_CONFIG);
	ValOutputCtrl=wss_base[TVE_OUTPUT_CONFIG];

	if(CompChan == CSVOUT_COMP_YVU) {
		ValOutputCtrl |= 0x8;
	} else {
		ValOutputCtrl &= ~0x8;
	}

	TVE_WriteRegDev8(DevBaseAddr, TVE_OUTPUT_CONFIG, ValOutputCtrl);

	return ( Error );
}
#endif

/*=============================== End CONFIG_WSS =====================================*/
#define     TTX_PACKETS_CACHE_MAX_SIZE     16*10
static struct {
	unsigned int	valid_flag;
	unsigned char	packet_data[TTX_LINESIZE];
} ttx_packets_cache[TTX_PACKETS_CACHE_MAX_SIZE];

static int ttx_read_ptr = 0;
static int ttx_write_ptr = 0;

int VBI_SetTxtPage( TTX_Page_t  *Page_p )
{
	TTX_Page_t TtxBuf;
	int i;
	unsigned long flags;

	copy_from_user(&TtxBuf, Page_p, sizeof(TTX_Page_t));

	if(TtxBuf.Valid_Lines_num > TTX_MAXLINES)
		TtxBuf.Valid_Lines_num  = TTX_MAXLINES;

	/* SUNHE: dump data! */
/*        ttx_dump_data(TtxBuf.Lines);*/

	spin_lock_irqsave(&orion15_df_lock, flags);
	for(i = 0; i<TtxBuf.Valid_Lines_num; i++) {
		ttx_packets_cache[ttx_write_ptr].valid_flag = 1;
		memcpy(ttx_packets_cache[ttx_write_ptr].packet_data, TtxBuf.Lines[i], TTX_LINESIZE);
		ttx_write_ptr++;

		if(ttx_write_ptr>=TTX_PACKETS_CACHE_MAX_SIZE) {
			ttx_write_ptr = 0;
		}
	}
	spin_unlock_irqrestore(&orion15_df_lock, flags);

	return 0;
}

int check_ttx_valid_packets(void)
{
	int count = 0;
	int read_ptr = 0;
	unsigned long flags;

	spin_lock_irqsave(&orion15_df_lock, flags);

	read_ptr = ttx_read_ptr;
	while(ttx_packets_cache[read_ptr].valid_flag ) {
		read_ptr++;
		if(read_ptr>=TTX_PACKETS_CACHE_MAX_SIZE) {
			read_ptr = 0;
		}
		count++;
	}

	spin_unlock_irqrestore(&orion15_df_lock, flags);

	//printk("check_ttx_valid_packets = %d\n", count);
	if(count>TTX_MAXLINES)
		count = TTX_MAXLINES;

	return(count);
}

unsigned int inv_word(unsigned int b)
{
	unsigned int n; 
	unsigned int i;

	for (i=0, n=0; i<32; i++)
		if ((b&(1<<i)) != 0)
			n |= (0x80000000>>i);
	return n;
}

//#define TTX_NO_INTR
#if defined(TTX_NO_INTR)

static struct timer_list vbi_timer;

#define k_send_times      3
unsigned char  k_send_Lines[k_send_times][TTX_MAXLINES][TTX_LINESIZE] = 
{
/*0*/ /*100*/
/*********************************line 0*********************************************************************/
0x40, 0xa8, 0xa8, 0xa8, 0x26, 0xa8, 0xa8, 0xa8, 0x1c, 0xa8, 0x80, 0x83, 0xae, 0xce, 
0x2f, 0xa7, 0x1f, 0x2f, 0x40, 0x8c, 0xd, 0xd, 0xe0, 0x2a, 0xae, 0xa7, 0x4, 0x4c, 
0xec, 0x4, 0xb3, 0x86, 0x9e, 0xc1, 0xd, 0x9d, 0x5d, 0x4c, 0x4c, 0x5d, 0x2c, 0xad, 

/*********************************line 1*********************************************************************/
0xe3, 0xa8, 0xe9, 0xb9, 0x80, 0xb0, 0x83, 0x4, 0xab, 0x4, 0xcb, 0x4, 0x2a, 0x4, 
0xa2, 0x4, 0x1a, 0x4, 0x2a, 0x4, 0x4, 0x4, 0xb3, 0x4, 0x83, 0x4, 0x92, 0x4, 
0x73, 0x4, 0x4, 0x4, 0x92, 0x4, 0x73, 0x4, 0x23, 0x4, 0xa2, 0x4, 0x1a, 0x4, 

/*********************************line 2*********************************************************************/
0x40, 0x40, 0x68, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 3*********************************************************************/
0xe3, 0x40, 0xe9, 0xb9, 0x20, 0x4, 0x4, 0x4, 0xc2, 0xf2, 0xb, 0x9b, 0x4a, 0x92, 
0xe3, 0x13, 0x2a, 0x4, 0x92, 0x73, 0x62, 0xf2, 0x4a, 0xb3, 0x83, 0x2a, 0x92, 0xf2, 
0x73, 0x4, 0xf2, 0x73, 0x4, 0x8c, 0xd, 0x2c, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 4*********************************************************************/
0x40, 0x92, 0x89, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 
0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 
0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 

/*********************************line 5*********************************************************************/
0xe3, 0x92, 0x20, 0xb9, 0x4, 0xc1, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x92, 0x73, 
0x23, 0xa2, 0x1a, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x92, 
0x73, 0x62, 0xf2, 0x4a, 0xb3, 0x83, 0x2a, 0x92, 0xf2, 0x73, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 6*********************************************************************/
0x40, 0x7a, 0x20, 0xb9, 0x61, 0xb3, 0x83, 0x92, 0x73, 0x4, 0x92, 0x73, 0x23, 0xa2, 
0x1a, 0x75, 0x75, 0x75, 0x8c, 0xd, 0xd, 0xc1, 0xb9, 0x80, 0x62, 0xf2, 0x4a, 0xa2, 
0xc2, 0x83, 0xcb, 0x2a, 0x5d, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 7*********************************************************************/
0xe3, 0x7a, 0x20, 0xb9, 0x61, 0x43, 0xab, 0xcb, 0x92, 0x73, 0xa2, 0xcb, 0xcb, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x8c, 0x8c, 0x8c, 0xc1, 0xb9, 0x20, 0xc2, 0x83, 0xb, 0x92, 
0x2a, 0x83, 0x32, 0x4, 0xc2, 0x92, 0x2a, 0x92, 0xa2, 0xcb, 0x75, 0xcd, 0xd, 0x8c, 

/*********************************line 8*********************************************************************/
0x40, 0x26, 0x29, 0xb9, 0x61, 0x73, 0xa2, 0xea, 0xcb, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x8c, 0x8c, 0x9d, 0xc1, 0xb9, 0x20, 0x43, 0x4a, 0x92, 0xcb, 
0x43, 0x83, 0x73, 0xa2, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0xcd, 0xd, 0x4c, 

/*********************************line 9*********************************************************************/
0xe3, 0x26, 0x29, 0xb9, 0x61, 0xcb, 0xb, 0xf2, 0x4a, 0x2a, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x8c, 0x6d, 0xd, 0xc1, 0xb9, 0x20, 0xcb, 0x9b, 0x23, 0x73, 
0xa2, 0x9b, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0xcd, 0x4c, 0xd, 

/*********************************line 10*********************************************************************/
0x40, 0xce, 0x20, 0xb9, 0x61, 0x20, 0x92, 0x73, 0x83, 0x73, 0xc2, 0xa2, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x4c, 0xd, 0xd, 0xc1, 0xb9, 0x20, 0xc2, 0x83, 0x73, 0x43, 
0xa2, 0x4a, 0x4a, 0x83, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0xcd, 0xcd, 0x6d, 

/*********************************line 11*********************************************************************/
0xe3, 0xce, 0xc1, 0xb9, 0x20, 0xea, 0xa2, 0x83, 0x2a, 0x13, 0xa2, 0x4a, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0xcd, 0xd, 0xd, 0xc1, 0xb9, 0x20, 0xb3, 0xa2, 0x32, 0x43, 
0xf2, 0xab, 0x4a, 0x73, 0xa2, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0xcd, 0x2c, 0x8c, 

/*********************************line 12*********************************************************************/
0x40, 0x1c, 0x29, 0xb9, 0x61, 0x2a, 0x83, 0x43, 0x4, 0xcb, 0xa2, 0x4a, 0x6b, 0x92, 
0xc2, 0xa2, 0x75, 0x75, 0xad, 0xd, 0xd, 0xc1, 0xb9, 0x20, 0x13, 0xf2, 0x43, 0x83, 
0x4a, 0x2a, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0xcd, 0xad, 0xcd, 

/*********************************line 13*********************************************************************/
0xe3, 0x1c, 0x29, 0xb9, 0x61, 0xe3, 0xa2, 0x73, 0xa2, 0x4a, 0x83, 0x32, 0x4, 0x92, 
0x73, 0x62, 0xf2, 0x75, 0x6d, 0xd, 0xd, 0xc1, 0xb9, 0x20, 0x83, 0x23, 0xa2, 0x32, 
0x83, 0x92, 0x23, 0xa2, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0xcd, 0xad, 0x1c, 

/*********************************line 14*********************************************************************/
0x40, 0xf4, 0x29, 0xb9, 0x61, 0x2a, 0x6b, 0x4, 0xb, 0x4a, 0xf2, 0xe3, 0x4a, 0x83, 
0xb3, 0xcb, 0x75, 0x75, 0x6d, 0xad, 0xd, 0xc1, 0xb9, 0x20, 0xb, 0xa2, 0x4a, 0x2a, 
0x13, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0xcd, 0x6d, 0xec, 
#if 0
/*********************************line 0*********************************************************************/
0x40, 0xa8, 0x40, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0x80, 0x83, 0xae, 0xce, 
0x2f, 0xa7, 0x1f, 0x2f, 0x40, 0x8c, 0xd, 0x8c, 0xe0, 0x2a, 0xae, 0xa7, 0x4, 0x4c, 
0xec, 0x4, 0xb3, 0x86, 0x9e, 0xc1, 0xd, 0x9d, 0x5d, 0x4c, 0x8c, 0x5d, 0x8c, 0xcd, 

#else

/*********************************line 15*********************************************************************/
0xe3, 0xf4, 0x29, 0xb9, 0x61, 0xc2, 0x83, 0xb, 0x2a, 0x92, 0xf2, 0x73, 0xcb, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x1c, 0xd, 0x8c, 0xc1, 0xb9, 0x20, 0x23, 0x83, 0x4a, 0xea, 
0x92, 0x73, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0xcd, 0x9d, 0xd, 

/*1*/
/*********************************line 16*********************************************************************/
0x40, 0xb, 0x29, 0xb9, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0xc1, 0xb9, 0x20, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 17*********************************************************************/
0xe3, 0xb, 0x20, 0xb9, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 18*********************************************************************/
0x40, 0xe3, 0x29, 0xb9, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 19*********************************************************************/
0xe3, 0xe3, 0x29, 0xb9, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 20*********************************************************************/
0x40, 0x31, 0x29, 0xb9, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 21*********************************************************************/
0xe3, 0x31, 0xe0, 0xb9, 0x4, 0x4, 0x80, 0xb3, 0x83, 0xd3, 0xa2, 0x4, 0x83, 0xab, 
0xcb, 0x2a, 0xa2, 0x1a, 0x2a, 0x4, 0x9b, 0xf2, 0xab, 0x4a, 0x4, 0x62, 0x92, 0x4a, 
0xcb, 0x2a, 0x4, 0xc2, 0x13, 0xf2, 0x92, 0xc2, 0xa2, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 22*********************************************************************/
0x40, 0xd9, 0xe0, 0xb9, 0x80, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x62, 0xf2, 0x4a, 0x4, 0x32, 0xf2, 0xc2, 0x83, 0x32, 0x4, 0x73, 0xa2, 0xea, 
0xcb, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 23*********************************************************************/
0xe3, 0xd9, 0x80, 0xb9, 0xe0, 0x4, 0x4, 0x4, 0x73, 0x83, 0x2a, 0x92, 0xf2, 0x73, 
0x83, 0x32, 0x4, 0x73, 0xa2, 0xea, 0xcb, 0x4, 0x92, 0x73, 0x23, 0xa2, 0x1a, 0x4, 
0xb, 0x83, 0xe3, 0xa2, 0x4, 0x8c, 0x4c, 0xd, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
/*101*/
/*********************************line 0*********************************************************************/
0x40, 0xa8, 0x40, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0x80, 0x83, 0xae, 0xce, 
0x2f, 0xa7, 0x1f, 0x2f, 0x40, 0x8c, 0xd, 0x8c, 0xe0, 0x2a, 0xae, 0xa7, 0x4, 0x4c, 
0xec, 0x4, 0xb3, 0x86, 0x9e, 0xc1, 0xd, 0x9d, 0x5d, 0x4c, 0x8c, 0x5d, 0x8c, 0xcd, 

/*********************************line 1*********************************************************************/
0xe3, 0xa8, 0x49, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 
0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 
0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 

/*********************************line 2*********************************************************************/
0x40, 0x40, 0xe0, 0xb9, 0xb0, 0x20, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x62, 0x4, 
0xab, 0x4, 0x32, 0x4, 0x32, 0x4, 0x4, 0x4, 0x92, 0x4, 0x73, 0x4, 0x23, 0x4, 
0xa2, 0x4, 0x1a, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 

/*********************************line 4*********************************************************************/
0x40, 0x92, 0x49, 0x3d, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 
0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 
0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x37, 

/*********************************line 5*********************************************************************/
0xe3, 0x92, 0x49, 0xad, 0xc1, 0xb3, 0x83, 0x92, 0x73, 0x4, 0x92, 0x73, 0x23, 0xa2, 
0x1a, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x8c, 0xd, 0xd, 0x49, 0x57, 

/*********************************line 6*********************************************************************/
0x40, 0x7a, 0x49, 0xad, 0xc1, 0x2a, 0xf2, 0xb, 0x4, 0x2a, 0xa2, 0x1a, 0x2a, 0x4, 
0x13, 0xa2, 0x32, 0xb, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x8c, 0xd, 0xad, 0x49, 0x57, 

/*********************************line 7*********************************************************************/
0xe3, 0x7a, 0x49, 0xad, 0xe0, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x49, 0x57, 

/*********************************line 8*********************************************************************/
0x40, 0x26, 0x49, 0xad, 0xe0, 0x43, 0xab, 0xcb, 0x92, 0x73, 0xa2, 0xcb, 0xcb, 0x4, 
0x73, 0xa2, 0xea, 0xcb, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x8c, 0x8c, 0x8c, 0xb5, 0x8c, 0x8c, 0x6d, 0x49, 0x57, 

/*2*/
/*********************************line 9*********************************************************************/
0xe3, 0x26, 0x49, 0xad, 0xc1, 0x73, 0xa2, 0xea, 0xcb, 0x4, 0xcb, 0xa2, 0x4a, 0x6b, 
0x92, 0xc2, 0xa2, 0x4, 0x92, 0x73, 0x23, 0xa2, 0x1a, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x8c, 0x8c, 0x9d, 0x49, 0x57, 

/*********************************line 10*********************************************************************/
0x40, 0xce, 0x49, 0xad, 0xe0, 0x73, 0x83, 0x2a, 0x92, 0xf2, 0x73, 0x83, 0x32, 0x4, 
0x73, 0xa2, 0xea, 0xcb, 0x4, 0x92, 0x73, 0x23, 0xa2, 0x1a, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x8c, 0x4c, 0xd, 0x49, 0x57, 

/*********************************line 11*********************************************************************/
0xe3, 0xce, 0x49, 0xad, 0xc1, 0x92, 0x73, 0x2a, 0xa2, 0x4a, 0x73, 0x83, 0x2a, 0x92, 
0xf2, 0x73, 0x83, 0x32, 0x4, 0x73, 0xa2, 0xea, 0xcb, 0x4, 0x92, 0x73, 0x23, 0xa2, 
0x1a, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x8c, 0x2c, 0xd, 0x49, 0x57, 

/*********************************line 12*********************************************************************/
0x40, 0x1c, 0x49, 0xad, 0xe0, 0xcb, 0xb, 0xf2, 0x4a, 0x2a, 0x4, 0x73, 0xa2, 0xea, 
0xcb, 0x4, 0x92, 0x73, 0x23, 0xa2, 0x1a, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x8c, 0x6d, 0xd, 0x49, 0x57, 

/*********************************line 13*********************************************************************/
0xe3, 0x1c, 0x49, 0xad, 0xc1, 0x23, 0xa2, 0x83, 0x62, 0x4, 0x64, 0x4, 0x13, 0x83, 
0x4a, 0x23, 0x4, 0xf2, 0x62, 0x4, 0x13, 0xa2, 0x83, 0x4a, 0x92, 0x73, 0xe3, 0x4, 
0x92, 0x73, 0x62, 0xf2, 0x75, 0x75, 0x75, 0x75, 0x75, 0x8c, 0x1c, 0xd, 0x49, 0x57, 

/*********************************line 14*********************************************************************/
0x40, 0xf4, 0x49, 0xad, 0xe0, 0xea, 0xa2, 0x83, 0x2a, 0x13, 0xa2, 0x4a, 0x4, 0x92, 
0x73, 0x23, 0xa2, 0x1a, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0xcd, 0xd, 0xd, 0x49, 0x57, 


/*********************************line 15*********************************************************************/
0xe3, 0xf4, 0x49, 0xad, 0xc1, 0x4a, 0x83, 0xc2, 0x92, 0x73, 0xe3, 0x4, 0x92, 0x73, 
0x23, 0xa2, 0x1a, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0xad, 0xd, 0xd, 0x49, 0x57, 

/*********************************line 16*********************************************************************/
0x40, 0xb, 0x49, 0xad, 0xe0, 0xe3, 0xa2, 0x73, 0xa2, 0x4a, 0x83, 0x32, 0x4, 0x92, 
0x73, 0x62, 0xf2, 0x4, 0x92, 0x73, 0x23, 0xa2, 0x1a, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x6d, 0xd, 0xd, 0x49, 0x57, 

/*********************************line 17*********************************************************************/
0xe3, 0xb, 0x49, 0xad, 0xc1, 0x13, 0xf2, 0x4a, 0xf2, 0xcb, 0xc2, 0xf2, 0xb, 0xa2, 
0xcb, 0x4, 0x92, 0x73, 0x23, 0xa2, 0x1a, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x6d, 0xd, 0x6d, 0x49, 0x57, 

/*********************************line 18*********************************************************************/
0x40, 0xe3, 0x49, 0xad, 0xe0, 0x2a, 0x6b, 0x4, 0xe3, 0xab, 0x92, 0x23, 0xa2, 0x4, 
0x92, 0x73, 0x23, 0xa2, 0x1a, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x6d, 0xad, 0xd, 0x49, 0x57, 

/*********************************line 19*********************************************************************/
0xe3, 0xe3, 0x49, 0xad, 0xc1, 0xcb, 0xc2, 0x92, 0xf4, 0x2a, 0xa2, 0xc2, 0x13, 0x4, 
0x73, 0xa2, 0xea, 0xcb, 0x4, 0x92, 0x73, 0x23, 0xa2, 0x1a, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x6d, 0x1c, 0xd, 0x49, 0x57, 

/*********************************line 20*********************************************************************/
0x40, 0x31, 0x49, 0xad, 0xe0, 0xcb, 0x13, 0xf2, 0xea, 0x43, 0x92, 0x5b, 0x4, 0x73, 
0xa2, 0xea, 0xcb, 0x4, 0x92, 0x73, 0x23, 0xa2, 0x1a, 0x75, 0x75, 0x75, 0x75, 0x75, 
0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x75, 0x6d, 0x1c, 0xd, 0x49, 0x57, 

/*********************************line 21*********************************************************************/
0xe3, 0x31, 0x49, 0xad, 0xc1, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x49, 0x57, 

/*********************************line 22*********************************************************************/
0x40, 0xd9, 0x49, 0xad, 0xc1, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 
0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x49, 0x57, 

/*********************************line 23*********************************************************************/
0xe3, 0xd9, 0x49, 0xb5, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 
0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 
0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x75, 

/*********************************line 23*********************************************************************/
0xe3, 0xd9, 0x49, 0xb5, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 
0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 
0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x75, 
#endif
};

/* TODO SUNHE: This is used for test only */
int cache_vbi_data(int packet_num)
{
	int i,j;
	int packets = 0;
	unsigned int * inv_value;
	unsigned long flags;

	spin_lock_irqsave(&orion15_df_lock, flags);

	while(1)
		if(wss_base[TVE_TTC_FW_ADDR] == 0)
			break;

	memcpy(ttx_buf, k_send_Lines[1], TTX_LINESIZE*TTX_MAXLINES);

	ttx_dump_data(ttx_buf);
	inv_value = ttx_buf;

	for(j=0;j<(TTX_MAXLINES*TTX_LINESIZE/4);j++) {
		*inv_value = inv_word(*inv_value);

		inv_value++;
	}
	spin_unlock_irqrestore(&orion15_df_lock, flags);

	return 0;
}
#endif

int cache_vbi_data2(int packet_num)
{
	int j;
	volatile unsigned int * inv_value;
	unsigned long flags;
	volatile unsigned char *tmp_buf;

	if (ttx_read_ptr == ttx_write_ptr)
		return -1;

	while(wss_base[TVE_TTC_FW_ADDR]);

	spin_lock_irqsave(&orion15_df_lock, flags);

	tmp_buf = ttx_buf;
	while (ttx_read_ptr != ttx_write_ptr) {
		memcpy((void *)tmp_buf, ttx_packets_cache[ttx_read_ptr].packet_data, TTX_LINESIZE);
		tmp_buf += TTX_LINESIZE; ttx_read_ptr++;

		if (ttx_read_ptr >= TTX_PACKETS_CACHE_MAX_SIZE)
			ttx_read_ptr = 0;
	}

	/* TODO SUNHE: Dump data! */
/*        ttx_dump_data(ttx_buf);*/

	/* SUNHE: Revert data */
	inv_value = (volatile unsigned int *)ttx_buf;
	for(j=0;j<(TTX_MAXLINES*TTX_LINESIZE/4);j++) {
		*inv_value = inv_word(*inv_value);
		inv_value++;
	}

	spin_unlock_irqrestore(&orion15_df_lock, flags);

	return 0;
}

int send_vbi_ttx_data(int field, int packet_num)
{
	int    Error = 0;
	unsigned long flags;
	u16 BitMask = 0xffff;
	int i;

	spin_lock_irqsave(&orion15_df_lock, flags);
	TVE_WriteRegDev16(DevBaseAddr, TVE_TTX_LINEDISABLE_BASE, 0xffff);
	TVE_WriteRegDev32(DevBaseAddr, TVE_TTC_FW_ADDR, dma_phy_addr); 

	for(i = 0; i<packet_num; i++)
		BitMask &= ~(1<<i);

	TVE_WriteRegDev16(DevBaseAddr, TVE_TTX_LINEDISABLE_BASE, BitMask); 
	spin_unlock_irqrestore(&orion15_df_lock, flags);

	return ( Error );
}

void vbi_ttx_sending(unsigned long data)
{
/*        ttx_dump_data(ttx_buf);*/
#if defined(TTX_NO_INTR)
	cache_vbi_data(16); 	
#else
	cache_vbi_data2(16);
#endif
	send_vbi_ttx_data(0, 16);

#if defined(TTX_NO_INTR)
	mod_timer(&vbi_timer, jiffies + msecs_to_jiffies(2000));
#endif

	return; 
}
EXPORT_SYMBOL(vbi_ttx_sending);

int __get_tveID(int video){
	return layer_output.vid_output[video];
}

static int orion15_df_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	unsigned int minor_id = iminor(inode);
	unsigned int reg_val;
	int tveid = 0;
	unsigned long flags;

	/* FIXME: SunHe exist for testing */
#if 1
	switch (minor_id) {
		case DF_MINOR_TVE0:
			tveid = 0;
			break;
		case DF_MINOR_TVE1:
			tveid = 1;
			break;
	}
#endif

	switch (cmd) {
		case CSTVE_IOC_BIND_GFX:
		{
			layer_output.gfx_output[arg] = (df_dev[minor_id].dev_type == DF_DEV_TVE0) ? 0:1;
			DEBUG_PRINTF("layer_output.gfx_output[%d] = %d\n",arg,layer_output.gfx_output[arg]);
			break;
		}
		case CSTVE_IOC_BIND_VID:
		{
			layer_output.vid_output[arg] = (df_dev[minor_id].dev_type == DF_DEV_TVE0) ? 0:1;
			DEBUG_PRINTF("layer_output.vid_output[%d] = %d\n",arg,layer_output.vid_output[arg]);
			break;
		}
		case CSTVE_IOC_SET_OUTPUT:
		{
			if(df_dev[minor_id].dev_type == DF_DEV_TVE0){
				tveid = 0;
				//DEBUG_PRINTF("444444\n");
				if(arg == OUTPUT_MODE_YPBPR){
					DFSetAnalogOutFmt(1, 0);
					//DEBUG_PRINTF("555555\n");
				}
				else if(arg == OUTPUT_MODE_RGB){
					//DFSetAnalogOutFmt(1, 1);
					if(layer_output.IsTV){
						DFSetDigitalOutFmt(0, 0);
						//DEBUG_PRINTF("666666\n");
					}
					else{
						DFSetDigitalOutFmt(0, 1);
						//DEBUG_PRINTF("777777\n");
					}
				}
				else{
					DEBUG_PRINTF("ERROR: No such mode %d in TVE%d\n",arg,tveid);
				}
			}
			else if(df_dev[minor_id].dev_type == DF_DEV_TVE1){
				tveid = 1;
				//DEBUG_PRINTF("111111\n");
				if(arg == OUTPUT_MODE_YPBPR){
					DFSetAnalogOutFmt(0, 0);
					DFSetAnalogOutFmt(1, 2);

					//DEBUG_PRINTF("222222\n");
				}
				else if(arg == OUTPUT_MODE_CVBS_SVIDEO){
					DFSetAnalogOutFmt(0, 0);
					//DEBUG_PRINTF("333333\n");
				}
				else{
					DEBUG_PRINTF("ERROR: No such mode %d in TVE%d\n",arg,tveid);
				}
			}
			else
				DEBUG_PRINTF("ERROR: No such device in orion_df. Minor = %d",minor_id);

			break;
		}
		case CSTVE_IOC_ENABLE:
		{	
			if(df_dev[minor_id].dev_type == DF_DEV_TVE0){
				tveid = 0;
				TVE0SetClk(df_dev[minor_id].tve_format, __Is_ColorBar);
				InitTVE0Raw(df_dev[minor_id].tve_format, __Is_ColorBar);
			}
			else if(df_dev[minor_id].dev_type == DF_DEV_TVE1){
				tveid = 1;
				TVE1SetClk(df_dev[minor_id].tve_format, __Is_ColorBar, __IsSysPll);
				InitTVE1Raw(df_dev[minor_id].tve_format, __Is_ColorBar, __IsSysPll);
			}
			else
				DEBUG_PRINTF("ERROR: No such device in orion_df. Minor = %d",minor_id);

			__df_update_start();
			DFSetOutIFVideoFmt(tveid, tve_2_df[df_dev[minor_id].tve_format]);
			__df_update_end();
			break;
		}
		case CSTVE_IOC_DISABLE:
		{
			if(df_dev[minor_id].dev_type == DF_DEV_TVE0){
				tveid = 0;
				tveid = CLK_Read(REG_CLK_FREQ_CTRL);
				tveid &= (~0x8);
				CLK_Write(REG_CLK_FREQ_CTRL, tveid);/* Disable TVE0_CLK. */
			}
			else if(df_dev[minor_id].dev_type == DF_DEV_TVE1){
				tveid = 1;
				tveid = CLK_Read(REG_CLK_FREQ_CTRL);
				tveid &= (~0x8);
				CLK_Write(REG_CLK_FREQ_CTRL, tveid);/* Disable TVE1_CLK. */
			}
			else
				DEBUG_PRINTF("ERROR: No such device in orion_df. Minor = %d",minor_id);

			break;
		}
		case CSTVE_IOC_SET_MODE:
		{
			struct df_output_pos hd2sd_out_pos;
			unsigned int changemode_ack = 0;
			unsigned char is_timeout = 0;
			unsigned long long time_start = 0;

			if(__Is_first){
				DFSetAnalogOutFmt(0, 0);//DAC0,TVE1,CVBS/S-VIDEO
				if(__VGA_Support){
					__setTVEncoderClk_TVE0(0, 1);

					DFSetAnalogOutFmt(1, 1);//DAC1,TVE0,RGB
					DFSetDigitalOutFmt(0, 1);//444

					__df_update_start();
					DFVideoEnaColorModulator(0, 1);
					DFVideoSetColorModulatorCoeff(0, YUV_HD_TO_RGB);
					__df_update_end();
				}
				else{
#if !defined(CONFIG_ARCH_ORION_CSM1201_J)
					DFSetAnalogOutFmt(1, 0);//DAC1,TVE0,YPBPR
					DFSetDigitalOutFmt(0, 0);//422
#else
					DFSetAnalogOutFmt(1, 2);//DAC1,TVE1,YPBPR
					DFSetDigitalOutFmt(1, 0);//422
#endif
					TVE0SetClk(TVOUT_MODE_576I, __Is_ColorBar);
					InitTVE0Raw(TVOUT_MODE_576I, __Is_ColorBar);
					TVE1SetClk(TVOUT_MODE_576I, __Is_ColorBar,__IsSysPll);
					InitTVE1Raw(TVOUT_MODE_576I, __Is_ColorBar,__IsSysPll);
				}

				__df_update_start();
				//DFOutIFEna(0, 1);
				//DFOutIFEna(1, 1);
				//if(__VGA_Support){
					//DFSetOutIFVideoFmt(0, DISP_RGB_1280X1024_60FPS);
				//}
				//else{
					//DFSetOutIFVideoFmt(0, DISP_YUV_PAL);
				//}
				DFSetOutIFVideoFmt(1, DISP_YUV_PAL);
				DFGfxEna(0, 1, __Is_TV);
				DFGfxEna(1, 0, __Is_TV);
				DFGfxSetColorKey(0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
				DFGfxColorKeyEna(0, 1);
				DFGfxSetColorKey(1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
				DFGfxColorKeyEna(1, 1);
				DFVideoEna(0, 1);
				if(__VGA_Support){
					DFSetBackGroud(0, 0x0, 0x0, 0x0);

					/* By KB KIm 2011.01.29 */
					DFOutIFClipCfg(0, 0, 16, 219, 224);
					// DFOutIFClipCfg(0, 0, 0, 255, 255);
				}
				else{
					DFSetBackGroud(0, 0x10, 0x80, 0x80);

					
					/* By KB KIm 2011.01.29 */
					DFOutIFClipCfg(0, 1, 16, 219, 224);
					// DFOutIFClipCfg(0, 1, 0, 255, 255);
				}
#if !defined(CONFIG_ARCH_ORION_CSM1201_J)
				DFVideoEna(1, 1);

				/* By KB KIm 2011.01.29 */
				DFOutIFClipCfg(1, 0, 16, 219, 224);
				// DFOutIFClipCfg(1, 0, 0, 255, 255);
#else
				DFVideoEna(1, 0);

				/* By KB KIm 2011.01.29 */
				DFOutIFClipCfg(1, 1, 16, 219, 224);
				// DFOutIFClipCfg(1, 1, 0, 255, 255);
#endif
				__df_update_end();

				//__Is_first = 0;
			}

			switch (arg)
			{
				case TVOUT_MODE_SECAM:
				case TVOUT_MODE_PAL_N:
				case TVOUT_MODE_PAL_M:
				case TVOUT_MODE_PAL_CN:
				case TVOUT_MODE_576I:
				case TVOUT_MODE_480I:
				case TVOUT_MODE_1080I30:
				case TVOUT_MODE_1080I25:
				case TVOUT_MODE_720P60: 
				case TVOUT_MODE_720P50:
				case TVOUT_MODE_1080P24:
				case TVOUT_MODE_1080P25:
				case TVOUT_MODE_1080P30:
				case TVOUT_MODE_480P:
				case TVOUT_MODE_576P:
				//case TVOUT_RGB_640X480_60FPS://duliguo
				{
					if((__Is_TV == 0)&&(__VGA_Support == 1)){
						DFSetAnalogOutFmt(1, 0);
						DFSetDigitalOutFmt(0, 0);
						__df_update_start();
						DFVideoEnaColorModulator(0, 0);
						DFSetBackGroud(0, 0x10, 0x80, 0x80);
						__df_update_end();
						__Is_TV = 1;
						__VGA_Support = 0;
					}
					break;
				}
				case TVOUT_RGB_848X480_60FPS:
				case TVOUT_RGB_800X480_60FPS:
				case TVOUT_RGB_640X480_60FPS:
				case TVOUT_RGB_800X600_60FPS://duliguo
				case TVOUT_RGB_800X600_72FPS:
				case TVOUT_RGB_1024X768_60FPS:
				{
					__df_update_start();
					DFVideoSetLayerAlpha(1, 0x0, 0, 0);
					DFVideoEna(1, 0);
					DFOutIFEna(1, 0);
					DFHD2SDCaptureEna(0, 0);
					__df_update_end();
					udelay(100);
					if((__Is_TV == 1)&&(__VGA_Support == 0)){
						DFSetAnalogOutFmt(1, 1);
						DFSetDigitalOutFmt(0, 1);
						__df_update_start();
						DFVideoEnaColorModulator(0, 1);
						DFVideoSetColorModulatorCoeff(0, YUV_SD_TO_RGB);
						DFSetBackGroud(0, 0x0, 0x0, 0x0);
						__df_update_end();
						__Is_TV = 0;
						__VGA_Support = 1;
					}
					break;
				}
				case TVOUT_RGB_1280X1024_50FPS:
				case TVOUT_RGB_1600X1000_60FPS:
				case TVOUT_RGB_1280X1024_60FPS:
				case TVOUT_RGB_1280X720_60FPS:
				{
					__df_update_start();
					DFHD2SDCaptureEna(0, 0);
					DFVideoSetLayerAlpha(1, 0x0, 0, 0);
					DFVideoEna(1, 0);
					DFOutIFEna(1, 0);
					__df_update_end();
					udelay(100);
					if((__Is_TV == 1)&&(__VGA_Support == 0)){
						DFSetAnalogOutFmt(1, 1);
						DFSetDigitalOutFmt(0, 1);
						__df_update_start();
						DFVideoEnaColorModulator(0, 1);
						DFVideoSetColorModulatorCoeff(0, YUV_HD_TO_RGB);
						DFSetBackGroud(0, 0x0, 0x0, 0x0);
						__df_update_end();
						__Is_TV = 0;
						__VGA_Support = 1;
					}
					break;
				}
				default:
				{
					if((__Is_TV == 0)&&(__VGA_Support == 1)){
						DFSetAnalogOutFmt(1, 0);
						DFSetDigitalOutFmt(0, 0);
						__df_update_start();
						DFVideoEnaColorModulator(0, 0);
						DFSetBackGroud(0, 0x10, 0x80, 0x80);
						__df_update_end();
						__Is_TV = 1;
						__VGA_Support = 0;
					}
				}
				break;
			}

//			DFHD2SDCaptureEna(0, 0);

			__df_calc_video_position(df_dev[minor_id].tve_format,arg,&hd2sd_out_pos);
			df_dev[minor_id].tve_format = arg;

			if(df_dev[minor_id].dev_type == DF_DEV_TVE0){
				tveid = 0;
				changemode_ack = video_read(0x23);
				time_start = get_jiffies_64();
				while((changemode_ack>>29)&0x7){
					changemode_ack = video_read(0x23);
					if((get_jiffies_64() - time_start) > 4){
						video_write(changemode_ack&0x1fffffff,0x23);
						DEBUG_PRINTF("ERROR1: Timeout\n");
						is_timeout = 1;
						break;
					}
				}

				if(is_timeout == 0){
					video_write(changemode_ack|0x20000000,0x23);
					time_start = get_jiffies_64();
					changemode_ack = video_read(0x23);
					while(((changemode_ack>>29)&0x7) != 0x2){
						if((get_jiffies_64() - time_start) > 50){
							video_write(changemode_ack|0x60000000,0x23);
							DEBUG_PRINTF("ERROR2: Timeout\n");
							is_timeout = 1;
							break;
						}
						changemode_ack = video_read(0x23);
					}
				}

				TVE0SetClk(df_dev[minor_id].tve_format, __Is_ColorBar);
				udelay(1);
				
				if(__Is_TV){
			 		InitTVE0Raw(df_dev[minor_id].tve_format, __Is_ColorBar);
				}
			}
			else if(df_dev[minor_id].dev_type == DF_DEV_TVE1){
				tveid = 1;
#if defined(CONFIG_ARCH_ORION_CSM1201_J)
				changemode_ack = video_read(0x23);

				time_start = get_jiffies_64();
				while((changemode_ack>>21)&0x7){
					changemode_ack = video_read(0x23);
					if((get_jiffies_64() - time_start) > 4){
						video_write(changemode_ack&0xff1fffff,0x23);
						DEBUG_PRINTF("ERROR1: Timeout\n");
						is_timeout = 1;
						break;
					}
				}

				if(is_timeout == 0){
					video_write(changemode_ack|0x200000,0x23);
					time_start = get_jiffies_64();
					changemode_ack = video_read(0x23);
					while(((changemode_ack>>21)&0x7) != 0x2){
						if((get_jiffies_64() - time_start) > 50){
							video_write(changemode_ack|0x600000,0x23);
							DEBUG_PRINTF("ERROR2: Timeout\n");
							is_timeout = 1;
							break;
						}
						changemode_ack = video_read(0x23);
					}
				}
#endif
				TVE1SetClk(df_dev[minor_id].tve_format, __Is_ColorBar, __IsSysPll);
				InitTVE1Raw(df_dev[minor_id].tve_format, __Is_ColorBar, __IsSysPll);
			}
			else
				DEBUG_PRINTF("ERROR: No such device in orion_df. Minor = %d\n",minor_id);

			__df_update_start();
			if(__Is_first){
				DFOutIFEna(0, 1);
			}
			DFSetOutIFVideoFmt(tveid, tve_2_df[df_dev[minor_id].tve_format]);
			__df_update_end();

			if(tveid == 0){
				if(is_timeout == 0){
					__df_update_start();
#if defined(DEFAULT_SCALER)
					DFVideoSetDefaultCoeff(0);
#else
					DFSetVideoScalerCfg(0, hd2sd_out_pos.src.right, hd2sd_out_pos.src.bottom, hd2sd_out_pos.dst.right, hd2sd_out_pos.dst.bottom);
#endif

					hd2sd_out_pos.src.left = 0;
					hd2sd_out_pos.src.right = 0;
					hd2sd_out_pos.src.top = 0;
					hd2sd_out_pos.src.bottom = 0;
					DFVideoSetSrcCrop(0, hd2sd_out_pos.src);
					DFVideoSetDispWin(0, hd2sd_out_pos.dst);
					__df_update_end();
					changemode_ack = video_read(0x23);
					video_write(changemode_ack|0x60000000,0x23);
				}
				else{
					changemode_ack = video_read(0x23);
					video_write(changemode_ack|0x60000000,0x23);
				}
			}
			else{
#if !defined(CONFIG_ARCH_ORION_CSM1201_J)
				hd2sd_out_pos.src.top = 0;
				hd2sd_out_pos.src.bottom = 0;
				hd2sd_out_pos.src.left = 0;
				hd2sd_out_pos.src.right = 0;
				__df_update_start();
#if defined(DEFAULT_SCALER)
				DFVideoSetDefaultCoeff(1);
#else
				DFSetVideoScalerCfg(1, hd2sd_out_pos.src.right, hd2sd_out_pos.src.bottom, hd2sd_out_pos.dst.right, hd2sd_out_pos.dst.bottom);
#endif
				DFVideoSetSrcCrop(1, hd2sd_out_pos.src);
				DFVideoSetDispWin(1, hd2sd_out_pos.dst);
				__df_update_end();
				__df_update_start();
				DFVideoSetLayerAlpha(1, 0xff, 0, 0);
				DFVideoEna(1, 1);
				__df_update_end();
#else
				hd2sd_out_pos.src.top = 0;
				hd2sd_out_pos.src.bottom = 0;
				hd2sd_out_pos.src.left = 0;
				hd2sd_out_pos.src.right = 0;
				__df_update_start();
#if defined(DEFAULT_SCALER)
				DFVideoSetDefaultCoeff(0);
#else
				DFSetVideoScalerCfg(0, hd2sd_out_pos.src.right, hd2sd_out_pos.src.bottom, hd2sd_out_pos.dst.right, hd2sd_out_pos.dst.bottom);
#endif
				DFVideoSetSrcCrop(0, hd2sd_out_pos.src);
				DFVideoSetDispWin(0, hd2sd_out_pos.dst);
				__df_update_end();
				__df_update_start();
				DFVideoSetLayerAlpha(0, 0xff, 0, 0);
				DFVideoEna(0, 1);
				__df_update_end();
#endif
			}
			
			
			if(__Is_first){
				__df_update_start();
				DFOutIFEna(1, 1);
				__df_update_end();
				udelay(900);
				__df_update_start();
				DFOutIFEna(1, 0);
				__df_update_end();
			}
			if((tveid == 0)&&(__Is_TV == 1)){
				switch (df_dev[minor_id].tve_format){
					case TVOUT_MODE_SECAM:
						is_frame = 0;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 576;
						Capture_width = 720;
						Scaler_width = 720;
						hd2sd_out_pos.dst.bottom = 576;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_SECAM;
						break;
					case TVOUT_MODE_PAL_CN:
						is_frame = 0;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 576;
						Capture_width = 720;
						Scaler_width = 720;
						hd2sd_out_pos.dst.bottom = 576;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_PAL_CN;
						break;
					case TVOUT_MODE_576I:
						is_frame = 0;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 576;
						Capture_width = 720;
						Scaler_width = 720;
						hd2sd_out_pos.dst.bottom = 576;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_576I;
						break;
					case TVOUT_MODE_PAL_N:
						is_frame = 0;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 576;
						Capture_width = 720;
						Scaler_width = 720;
						hd2sd_out_pos.dst.bottom = 576;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_PAL_N;
						break;
					case TVOUT_MODE_480I:
						is_frame = 0;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 480;
						Capture_width = 720;
						Scaler_width = 720;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_MODE_PAL_M:
						is_frame = 0;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 480;
						Capture_width = 720;
						Scaler_width = 720;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_PAL_M;
						break;
					case TVOUT_MODE_1080I25:
						is_frame = 0;
						is_hd = 0;
						by_pass_sca = 0;
						Capture_height = 704;
						Capture_width = 960;
						Scaler_width = 1920;
						hd2sd_out_pos.dst.bottom = 576;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_576I;
						break;
					case TVOUT_MODE_1080I30:
						is_frame = 0;
						is_hd = 0;
						by_pass_sca = 0;
						Capture_height = 704;
						Capture_width = 960;
						Scaler_width = 1920;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_MODE_576P:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 576;
						Capture_width = 720;
						Scaler_width = 720;
						hd2sd_out_pos.dst.bottom = 576;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_576I;
						break;
					case TVOUT_MODE_480P:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 480;
						Capture_width = 720;
						Scaler_width = 720;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_MODE_720P50:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 0;
						Capture_height = 704;
						Capture_width = 640;
						Scaler_width = 1280;
						hd2sd_out_pos.dst.bottom = 576;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_576I;
						break;
					case TVOUT_MODE_1080P24:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 0;
						Capture_height = 704;
						Capture_width = 1024;
						Scaler_width = 1920;
						hd2sd_out_pos.dst.bottom = 576;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_576I;
						break;
					case TVOUT_MODE_720P60:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 0;
						Capture_height = 704;
						Capture_width = 640;
						Scaler_width = 1280;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_RGB_640X480_60FPS:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 480;
						Capture_width = 640;
						Scaler_width = 640;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 640;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_RGB_800X600_60FPS://duliguo
					case TVOUT_RGB_800X600_72FPS:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 600;
						Capture_width = 800;
						Scaler_width = 800;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_RGB_1024X768_60FPS:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 0;
						Capture_height = 704;
						Capture_width = 1008;
						Scaler_width = 1024;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_RGB_1280X1024_50FPS:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 0;
						Capture_height = 704;
						Capture_width = 1024;
						Scaler_width = 1280;
						hd2sd_out_pos.dst.bottom = 576;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_576I;
						break;
					case TVOUT_RGB_1280X1024_60FPS:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 0;
						Capture_height = 704;
						Capture_width = 1024;
						Scaler_width = 1280;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_RGB_1600X1000_60FPS:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 0;
						Capture_height = 704;
						Capture_width = 1024;
						Scaler_width = 1600;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_RGB_1280X720_60FPS:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 0;
						Capture_height = 704;
						Capture_width = 1024;
						Scaler_width = 1280;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_RGB_848X480_60FPS:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 480;
						Capture_width = 848;
						Scaler_width = 848;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						//df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					case TVOUT_RGB_800X480_60FPS:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 480;
						Capture_width = 800;
						Scaler_width = 800;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 720;
						//df_dev[minor_id+1].tve_format = TVOUT_MODE_480I;
						break;
					default:
						is_frame = 1;
						is_hd = 0;
						by_pass_sca = 1;
						Capture_height = 480;
						Capture_width = 640;
						Scaler_width = 1024;
						hd2sd_out_pos.dst.bottom = 480;
						hd2sd_out_pos.dst.right = 640;
						df_dev[minor_id+1].tve_format = TVOUT_MODE_576I;
						DEBUG_PRINTF("kernel: TVOUT_MODE_576I\n");
						break;
				}

				TVE1SetClk(df_dev[minor_id+1].tve_format, __Is_ColorBar, __IsSysPll);

				__df_update_start();
				DFSetOutIFVideoFmt(tveid+1, tve_2_df[df_dev[minor_id+1].tve_format]);
				__df_update_end();

				InitTVE1Raw(df_dev[minor_id+1].tve_format, __Is_ColorBar, __IsSysPll);

				hd2sd_out_pos.dst.top = 0;
				hd2sd_out_pos.dst.left = 0;

				hd2sd_out_pos.src.top = 0;
				hd2sd_out_pos.src.bottom = Capture_height;
				hd2sd_out_pos.src.left = 0;
				hd2sd_out_pos.src.right = Capture_width;

				__df_update_start();
#if defined(DEFAULT_SCALER)
				DFVideoSetDefaultCoeff(1);
#else
				DFSetVideoScalerCfg(1, Capture_width, Capture_height, hd2sd_out_pos.dst.right, hd2sd_out_pos.dst.bottom);
#endif
				DFVideoSetSrcCrop(1, hd2sd_out_pos.src);
				DFVideoSetDispWin(1, hd2sd_out_pos.dst);
				DFVideoSetLayerAlpha(1, 0xff, 0, 0);
				DFVideoEna(1, 1);
				__df_update_end();
				__df_update_start();
				DFHD2SDConfig();
				__df_update_end();

				__df_update_start();
#if !defined(CONFIG_ARCH_ORION_CSM1201_J)
				DFHD2SDCaptureEna(1, 0);
#else
                DFHD2SDCaptureEna(0, 0);
#endif
				__df_update_end();
			}
			
			__Is_first = 0;
			break;
		}
		case CSTVE_IOC_GET_MODE:
		{
			__put_user(df_dev[minor_id].tve_format, (int __user *) arg);
			break;
		}

		/* SunHe added 2008/11/11 15:37 */
		case CSTVE_IOC_SET_WHILE_LEVEL:
			spin_lock_irqsave(&orion15_df_lock, flags);
			TVE_Write(tveid, 0x27, (arg>>8)&0x03);
			TVE_Write(tveid, 0x28, arg&0xff);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSTVE_IOC_GET_WHILE_LEVEL:
			spin_lock_irqsave(&orion15_df_lock, flags);
			reg_val = TVE_Read(tveid, 0x27); 
			reg_val <<=8;
			reg_val |= TVE_Read(tveid, 0x28);
			reg_val &=0x3ff; 
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			__put_user(reg_val, (int __user *) arg); 
			break;

		case CSTVE_IOC_SET_BLACK_LEVEL:
			spin_lock_irqsave(&orion15_df_lock, flags);
			TVE_Write(tveid, 0x29, (arg>>8)&0x03);
			TVE_Write(tveid, 0x2a, arg&0xff);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSTVE_IOC_GET_BLACK_LEVEL:
			spin_lock_irqsave(&orion15_df_lock, flags);
			reg_val = TVE_Read(tveid, 0x29);
			reg_val <<=8;
			reg_val |= TVE_Read(tveid, 0x2a);
			reg_val &=0x3ff; 
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			__put_user(reg_val, (int __user *) arg); 
			break;

		case CSTVE_IOC_SET_STURATION_LEVEL:
			spin_lock_irqsave(&orion15_df_lock, flags);
			TVE_Write(tveid, 0x23, arg&0xff);
			TVE_Write(tveid, 0x24, arg&0xff);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSTVE_IOC_GET_STURATION_LEVEL:
			spin_lock_irqsave(&orion15_df_lock, flags);
			reg_val = TVE_Read(tveid, 0x23);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			__put_user(reg_val, (int __user *) arg); 
			break;
		/* SunHe added 2008/11/11 15:37 */
		
		case CSDF_IOC_DISP_ON:// not needed in 1201
			spin_lock_irqsave(&orion15_df_lock, flags);
			//do nothing
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSDF_IOC_DISP_OFF:// not needed in 1201
			spin_lock_irqsave(&orion15_df_lock, flags);
			//do nothing
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

//1  GFX 
		case CSDF_IOC_DISP_GFX_ON://ok
			spin_lock_irqsave(&orion15_df_lock, flags);
			//DF_Gfx_Enable(0, arg, arg);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSDF_IOC_DISP_GFX_OFF://ok
			spin_lock_irqsave(&orion15_df_lock, flags);
			//DF_Gfx_Disable(arg);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

//1 Compositor
		case CSDF_IOC_Z_ORDER:  //ok
			spin_lock_irqsave(&orion15_df_lock, flags);
			//DF_COMP_SetZOrder(int OutIFId, int Gfx1ZOrder, int Gfx2ZOrder, int Video1ZOrder, int Video2ZOrder);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSDF_IOC_DISP_SETBGCOLOR://ok
			spin_lock_irqsave(&orion15_df_lock, flags);
			//DF_COMP_SetBackGroud(int OutIFId, unsigned char BGY, unsigned char BGU, unsigned char BGV);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

//1 Video
		case CSDF_IOC_DISP_VID_ON://ok
			spin_lock_irqsave(&orion15_df_lock, flags);
			__df_update_start();
			DFVideoEna(0, 1); // video layer 0
			__df_update_end();
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSDF_IOC_DISP_VID_OFF://ok
			spin_lock_irqsave(&orion15_df_lock, flags);
			__df_update_start();
			DFVideoEna(0, 0); // video layer 0
			__df_update_end();
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSDF_IOC_DISP_POS://not used
		{
			unsigned int changemode_ack = 0;
			unsigned char is_timeout = 0;
			unsigned long long time_start = 0;

                	if (copy_from_user(&out_pos, (void *)arg, sizeof(struct df_output_pos)))
				return -EFAULT;

			spin_lock_irqsave(&orion15_df_lock, flags);

			changemode_ack = df_read(dfreg_outif[tveid][5]);
			time_start = get_jiffies_64();
			while((changemode_ack>>6)&0x3){
				if((get_jiffies_64() - time_start) > 250){
						df_write(dfreg_outif[tveid][5], changemode_ack&0x3f);
						DEBUG_PRINTF("ERROR CSDF_IOC_DISP_POS 1: Timeout\n");
						is_timeout = 1;
						break;
				}
				changemode_ack = df_read(dfreg_outif[tveid][5]);
			}				

			if(is_timeout == 0){
				df_write(dfreg_outif[tveid][5], changemode_ack|0x40);
				time_start = get_jiffies_64();
				changemode_ack = df_read(dfreg_outif[tveid][5]);			
				while(((changemode_ack>>6)&0x3) != 0x2){
					if((get_jiffies_64() - time_start) > 250){
						df_write(dfreg_outif[tveid][5], changemode_ack&0x3f);
						DEBUG_PRINTF("ERROR CSDF_IOC_DISP_POS 2: Timeout\n");
						is_timeout = 1;
						break;
					}
					changemode_ack = df_read(dfreg_outif[tveid][5]);
				}
			}
			
			__df_update_start();
#if defined(DEFAULT_SCALER)
			DFVideoSetDefaultCoeff(0);
#else
			DFSetVideoScalerCfg(0, out_pos.src.right, out_pos.src.bottom, out_pos.dst.right, out_pos.dst.bottom);
#endif
			DFVideoSetSrcCrop(0, out_pos.src);
			DFVideoSetDispWin(0, out_pos.dst);
			__df_update_end();

			if(is_timeout == 0){
				changemode_ack = df_read(dfreg_outif[tveid][5]);
				df_write(dfreg_outif[tveid][5], changemode_ack | 0xc0);
			}
			else{
				changemode_ack = df_read(dfreg_outif[tveid][5]);
				df_write(dfreg_outif[tveid][5], changemode_ack & 0x3f);
			}
			
			spin_unlock_irqrestore(&orion15_df_lock, flags);

			break;
		}
		case CSDF_IOC_DISP_VID_ALPHA:
			spin_lock_irqsave(&orion15_df_lock, flags);
			__df_update_start();
			DFVideoSetLayerAlpha(0, arg, 0x00, 0x00); // setting alpha of video layer only.
			__df_update_end();
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSDF_IOC_DISP_VIDFMT:
			spin_lock_irqsave(&orion15_df_lock, flags);
			//__df_disp_src_fmt(0, arg);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSDF_IOC_DISP_SETMODE:
			spin_lock_irqsave(&orion15_df_lock, flags);
			//video_disp_out_fmt(0, arg);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

#if 0
		case CSDF_IOC_WSS_CTRL:
			spin_lock_irqsave(&orion_tve_lock, flags);
			WSS_Ctrl((VBI_WssStandard_t)arg);
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;

		case CSDF_IOC_WSS_SETCONFIG:
			spin_lock_irqsave(&orion_tve_lock, flags);
			WSS_SetConfig((VBI_WssStandard_t)arg);
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;
		case CSDF_IOC_WSS_SETINFO:
			spin_lock_irqsave(&orion_tve_lock, flags);
			VBI_SetWssInfo((VBI_WssInfo_t *)arg);
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;
#endif
		case CSDF_IOC_TTX_CTRL:
			spin_lock_irqsave(&orion15_df_lock, flags);
			ttx_control(arg);
			Is_TTX = arg;
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSDF_IOC_TTX_SETCONFIG:
			spin_lock_irqsave(&orion15_df_lock, flags);
			ttx_setconfig(arg);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSDF_IOC_TTX_SETINFO:
			spin_lock_irqsave(&orion15_df_lock, flags);
			VBI_SetTxtPage((TTX_Page_t *)arg);
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSTVE_IOC_GET_BIND_INF:
			spin_lock_irqsave(&orion15_df_lock, flags);
			if (copy_to_user((void *) arg, (void *) &layer_output, sizeof(layer_output))) {
				spin_unlock_irqrestore(&orion15_df_lock, flags);
				return -EFAULT;
			}
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		case CSTVE_IOC_SET_COMP_CHAN:
			spin_lock_irqsave(&orion15_df_lock, flags);
			{
				unsigned char reg_val = 0;

				reg_val = __tve1_reg_read(0x4);
				if(CSVOUT_COMP_YVU== arg){
					reg_val |= 0x8;
					DFSetAnalogOutFmt(0, 0);
					DFSetAnalogOutFmt(1, 2);
				}
				else if(CSVOUT_COMP_RGB== arg){
					reg_val &= ~0x8;
					DFSetAnalogOutFmt(0, 0);
					DFSetAnalogOutFmt(1, 2);
				}
				_tve1_reg_write(0x4, reg_val);
			}
			spin_unlock_irqrestore(&orion15_df_lock, flags);
			break;

		default:
			break;
	}

	return 0;
}

static int df_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	const char *cmd_line = buffer;;
	
	if (strncmp("rl", cmd_line, 2) == 0) {
		int addr;
		addr = simple_strtol(&buffer[3], NULL, 16);
		printk(" addr[0x%08x] = 0x%08x \n", addr, df_read(addr));
	}
	else if (strncmp("wl", cmd_line, 2) == 0) {
		int addr, val;
		addr = simple_strtol(&buffer[3], NULL, 16);
		val = simple_strtol(&buffer[12], NULL, 16);
		__df_update_start();
		df_write(addr, val);
		__df_update_end();
	}
	else if (strncmp("rb", cmd_line, 2) == 0) {
		int addr, val;
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = TVE_Read(0, addr);
		printk(" 0x%02x = 0x%02x \n", addr, val);
	}
	else if (strncmp("wb", cmd_line, 2) == 0) {
		int addr, val;
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = simple_strtol(&cmd_line[6], NULL, 16);
		__df_update_start();
		TVE_Write(0, addr, val);
		__df_update_end();
	}

	else if (strncmp("rcb", cmd_line, 3) == 0) {
		int addr, val;
		addr = simple_strtol(&cmd_line[4], NULL, 16);
		val = TVE_Read(1, addr);
		printk(" 0x%02x = 0x%02x \n", addr, val);
	}
	else if (strncmp("wcb", cmd_line, 3) == 0) {
		int addr, val;
		addr = simple_strtol(&cmd_line[4], NULL, 16);
		val = simple_strtol(&cmd_line[7], NULL, 16);
		__df_update_start();
		TVE_Write(1, addr, val);
		__df_update_end();
	}
	////////////
	else if (strncmp("bg", cmd_line, 2) == 0) {
		__df_update_start();
		DFSetBackGroud(0, 0x00, 0x00, 0xff);
		__df_update_end();
	}
	else if(strncmp("disp",cmd_line,4) == 0){
		unsigned int val1 = 0;
		unsigned int val2 = 0;
		
		printk("df tve test\n");
		val1 = simple_strtol(&cmd_line[5], NULL, 16);
		printk("val1 = %d\n",val1);
		val2 = simple_strtol(&cmd_line[7], NULL, 16);
		printk("val2 = %d\n",val2);

		DFSetAnalogOutFmt(0, 0);//DAC0,TVE1,CVBS/S-VIDEO
		DFSetAnalogOutFmt(1, 2);//DAC1,TVE0,YPBPR
		TVE1SetClk(val1, val2,0);
		InitTVE1Raw(val1, val2,0);
	}
	else if((strncmp("outif",cmd_line,5) == 0)){
		unsigned int val1 = 0;

		printk("df_outif test\n");
		val1 = simple_strtol(&cmd_line[6], NULL, 16);
		printk("val1 = %d\n",val1);

		__df_update_start();
		DFSetOutIFVideoFmt(1, val1);
		DFGfxEna(0, 1, __Is_TV);
		__df_update_end();
	}
	else if((strncmp("dumpreg",cmd_line,7) == 0)){
		unsigned int count = 0;

		printk(" REG_CLK_FREQ_CTRL addr[0x%08x] = 0x%08x \n",REG_CLK_FREQ_CTRL,CLK_Read(REG_CLK_FREQ_CTRL));
		printk(" IDS_PCTL_LO addr[0x%08x] = 0x%08x \n",IDS_PCTL_LO,CLK_Read(IDS_PCTL_LO));
		printk(" IDS_OPROC_LOCTRL addr[0x%08x] = 0x%08x \n",IDS_OPROC_LOCTRL,__pin_mux0_read());
		printk(" IDS_OPROC_HICTRL addr[0x%08x] = 0x%08x \n",IDS_OPROC_HICTRL,__pin_mux1_read());

		for(count = 0; count < 0x300; count +=4){
			switch(count){
				case 0:
					printk("\n DISP_REG_COMMON\n");
					break;
				case 0x40:
					printk("\n DISP_REG_GFX1\n");
					break;
				case 0x80:
					printk("\n DISP_REG_GFX2\n");
					break;
				case 0xc0:
					printk("\n DISP_REG_VIDEO1\n");
					break;
				case 0x140:
					printk("\n DISP_REG_VIDEO2\n");
					break;
				case 0x1c0:
					printk("\n DISP_REG_COMP1\n");
					break;
				case 0x200:
					printk("\n DISP_REG_COMP2\n");
					break;
				case 0x240:
					printk("\n DISP_REG_HD2SD\n");
					break;
				case 0x280:
					printk("\n DISP_REG_OUTIF1\n");
					break;
				case 0x2c0:
					printk("\n DISP_REG_OUTIF2\n");
					break;
			}
			printk(" addr[0x%08x] = 0x%08x \n", 0x41800000 + count, df_read(0x41800000+count));
		}
	
	}
	else if((strncmp("audio",cmd_line,5) == 0)){
		DFSetAnalogOutFmt(0, 0);
		DFSetAnalogOutFmt(1, 0);
	}
	return count;
}


static int oriondf_open(struct inode *inode, struct file *file)
{
	unsigned int minor_id = iminor(inode);
	
	spin_lock_init(&(df_dev[minor_id].spin_lock));
	df_dev[minor_id].enable = 1;

	DEBUG_PRINTF("%s was opened\n",df_dev[minor_id].name);
	
	return 0;
}

static int oriondf_release(struct inode *inode, struct file *file)
{
	unsigned int minor_id = iminor(inode);

	df_dev[minor_id].enable = 0;

	DEBUG_PRINTF("%s was released\n",df_dev[minor_id].name);

	return 0;
}

/*
 * Architectures vary in how they handle caching for addresses
 * outside of main memory.
 *
 */
static inline int uncached_access(struct file *file, unsigned long addr)
{
#if defined(__i386__)
	/*
	 * On the PPro and successors, the MTRRs are used to set
	 * memory types for physical addresses outside main memory,
	 * so blindly setting PCD or PWT on those pages is wrong.
	 * For Pentiums and earlier, the surround logic should disable
	 * caching for the high addresses through the KEN pin, but
	 * we maintain the tradition of paranoia in this code.
	 */
	if (file->f_flags & O_SYNC)
		return 1;
 	return !( test_bit(X86_FEATURE_MTRR, boot_cpu_data.x86_capability) ||
		  test_bit(X86_FEATURE_K6_MTRR, boot_cpu_data.x86_capability) ||
		  test_bit(X86_FEATURE_CYRIX_ARR, boot_cpu_data.x86_capability) ||
		  test_bit(X86_FEATURE_CENTAUR_MCR, boot_cpu_data.x86_capability) )
	  && addr >= __pa(high_memory);
#elif defined(__x86_64__)
	/* 
	 * This is broken because it can generate memory type aliases,
	 * which can cause cache corruptions
	 * But it is only available for root and we have to be bug-to-bug
	 * compatible with i386.
	 */
	if (file->f_flags & O_SYNC)
		return 1;
	/* same behaviour as i386. PAT always set to cached and MTRRs control the
	   caching behaviour. 
	   Hopefully a full PAT implementation will fix that soon. */	   
	return 0;
#elif defined(CONFIG_IA64)
	/*
	 * On ia64, we ignore O_SYNC because we cannot tolerate memory attribute aliases.
	 */
	return !(efi_mem_attributes(addr) & EFI_MEMORY_WB);
#else
	/*
	 * Accessing memory above the top the kernel knows about or through a file pointer
	 * that was marked O_SYNC will be done non-cached.
	 */
	if (file->f_flags & O_SYNC)
		return 1;
	return addr >= __pa(high_memory);
#endif
}

static int oriondf_mmap(struct file *filp, struct vm_area_struct *vma)
{
#if defined(__HAVE_PHYS_MEM_ACCESS_PROT)
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;

	vma->vm_page_prot = phys_mem_access_prot(file, offset,
						 vma->vm_end - vma->vm_start,
						 vma->vm_page_prot);
#elif defined(pgprot_noncached)
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	int uncached;

	uncached = uncached_access(filp, offset);
	if (uncached)
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff,
			    vma->vm_end-vma->vm_start,
			    vma->vm_page_prot))
		return -EAGAIN;
	return 0;
}

static struct file_operations orion15_df_fops = {
	.owner	= THIS_MODULE,
	.open	= oriondf_open,
	.release	= oriondf_release,
	.ioctl		= orion15_df_ioctl,
	.mmap	= oriondf_mmap,
};

#if 0
static struct miscdevice orion15_df_miscdev = {
	MISC_DYNAMIC_MINOR,
	"orion_df",
	&orion15_df_fops
};
#endif

static struct proc_dir_entry *orion15_df_proc_entry = NULL;

static void csm_df_setup_dev(dev_t df_dev, char * dev_name)
{
	//devfs_mk_cdev(df_dev, S_IFCHR | S_IRUGO | S_IWUSR, dev_name);
    class_simple_device_add(csm_df_class, df_dev, NULL, dev_name); 

}


static int __init orion15_df_init(void)
{
	int i = 0;

	CLK_Write(IDS_PCTL_HI, 0x00000620);/* Majia add here. Set video PLL clock :288M*/

	if (register_chrdev(DF_MAJOR, "csm_df", &orion15_df_fops)){
		__df_mem_destroy();
		return -ENODEV;
	}

	csm_df_class = class_simple_create(THIS_MODULE,"csm_df");
	if (IS_ERR(csm_df_class)){
		printk(KERN_ERR "DF: class create failed.\n");
		__df_mem_destroy();
		return -ENODEV;
	}

	csm_df_setup_dev(MKDEV(DF_MAJOR, DF_MINOR_GFX0),  "csm_df_gfx0");
	csm_df_setup_dev(MKDEV(DF_MAJOR, DF_MINOR_GFX1),  "csm_df_gfx1");
	csm_df_setup_dev(MKDEV(DF_MAJOR, DF_MINOR_VID0),  "csm_df_video0");
	csm_df_setup_dev(MKDEV(DF_MAJOR, DF_MINOR_VID1),  "csm_df_video1");
	csm_df_setup_dev(MKDEV(DF_MAJOR, DF_MINOR_OUT0),  "csm_df_output0");
	csm_df_setup_dev(MKDEV(DF_MAJOR, DF_MINOR_OUT1),  "csm_df_output1");
	csm_df_setup_dev(MKDEV(DF_MAJOR, DF_MINOR_TVE0),  "csm_df_tvout0");
	csm_df_setup_dev(MKDEV(DF_MAJOR, DF_MINOR_TVE1),  "csm_df_tvout1");

	orion15_df_proc_entry = create_proc_entry("df15_io", 0, NULL);
	if (NULL != orion15_df_proc_entry) {
		orion15_df_proc_entry->write_proc = &df_proc_write;
	}

	for(i = 0; i<8; i++){
		df_dev[i].dev_minor = i;
		df_dev[i].dev_type = i;
		df_dev[i].enable = 0;
		df_dev[i].tve_format = TVOUT_MODE_576I;
		switch(i){
			case 0:
				df_dev[i].name = "csm_df_gfx0";
				break;
			case 1:
				df_dev[i].name = "csm_df_gfx1";
				break;
			case 2:
				df_dev[i].name = "csm_df_video0";
				break;
			case 3:
				df_dev[i].name = "csm_df_video1";
				break;
			case 4:
				df_dev[i].name = "csm_df_output0";
				break;
			case 5:
				df_dev[i].name = "csm_df_output1";
				break;
			case 6:
				df_dev[i].name = "csm_df_tvout0";
				break;
			case 7:
				df_dev[i].name = "csm_df_tvout1";
				break;
		}
	}

#if 0
	DFSetAnalogOutFmt(0, 0);//DAC0,TVE1,CVBS/S-VIDEO
	if(__VGA_Support){
		__setTVEncoderClk_TVE0(0, 1);
		
		DFSetAnalogOutFmt(1, 1);//DAC1,TVE0,RGB
		DFSetDigitalOutFmt(0, 1);//444

		__df_update_start();
		DFVideoEnaColorModulator(0, 1);
		DFVideoSetColorModulatorCoeff(0, YUV_HD_TO_RGB);
		__df_update_end();
	}
	else{
#if !defined(CONFIG_ARCH_ORION_CSM1201_J)
		DFSetAnalogOutFmt(1, 0);//DAC1,TVE0,YPBPR
		DFSetDigitalOutFmt(0, 0);//422
#else
		DFSetAnalogOutFmt(1, 2);//DAC1,TVE1,YPBPR
		DFSetDigitalOutFmt(0, 0);//422
#endif
		TVE0SetClk(TVOUT_MODE_576I, __Is_ColorBar);
		InitTVE0Raw(TVOUT_MODE_576I, __Is_ColorBar);
		TVE1SetClk(TVOUT_MODE_576I, __Is_ColorBar,__IsSysPll);
		InitTVE1Raw(TVOUT_MODE_576I, __Is_ColorBar,__IsSysPll);
	}

	__df_update_start();
	DFOutIFEna(0, 1);
	DFOutIFEna(1, 1);
	if(__VGA_Support){
		DFSetOutIFVideoFmt(0, DISP_RGB_1280X1024_60FPS);
	}
	else{
		DFSetOutIFVideoFmt(0, DISP_YUV_PAL);
	}
	DFSetOutIFVideoFmt(1, DISP_YUV_PAL);
//	DFGfxEna(0, 1, __Is_TV);
	DFGfxEna(1, 1, __Is_TV);
	DFVideoEna(0, 1);
	if(__VGA_Support){
		// DFOutIFClipCfg(0, 0, 16, 219, 224);
		DFOutIFClipCfg(0, 0, 0, 255, 255);
	}
	else{
		// DFOutIFClipCfg(0, 1, 16, 219, 224);
		DFOutIFClipCfg(0, 1, 0, 255, 255);
	}
#if !defined(CONFIG_ARCH_ORION_CSM1201_J)
	DFVideoEna(1, 1);
	// DFOutIFClipCfg(1, 0, 16, 219, 224);
	DFOutIFClipCfg(1, 0, 0, 255, 255);
#else
	DFVideoEna(1, 0);
	// DFOutIFClipCfg(1, 1, 16, 219, 224);
	DFOutIFClipCfg(1, 1, 0, 255, 255);
#endif
	__df_update_end();
#endif
	printk(KERN_INFO "%s: Orion Display feeder driver was initialized, at address@[phyical addr = %08x, size = %x] \n", 
		"orion15_df", DISP_REG_BASE, DISP_REG_SIZE);
	printk(KERN_INFO "%s: Orion TVE0 driver was initialized, at address@[phyical addr = %08x, size = %x] \n", 
		"orion15_df", TVE0_REG_BASE, TVE0_REG_SIZE);
	printk(KERN_INFO "%s: Orion TVE1 driver was initialized, at address@[phyical addr = %08x, size = %x] \n", 
		"orion15_df", TVE1_REG_BASE, TVE1_REG_SIZE);

	printk("pinmux0 : 0x%x, pinmux1: 0x%x\n",__pin_mux0_read(),__pin_mux1_read());

	{
		pool = dma_pool_create ("dpool", NULL,
					1024, 0, 0); 	/* XXX ?? */
		ttx_buf = dma_pool_alloc (pool, SLAB_ATOMIC, &dma_phy_addr);
		if (ttx_buf == NULL)
			printk("ERROR: dma_pool_alloc ERROR!\n");
		printk("dma_pool_alloc: dma_phy_addr: %x, ttx_buf: %p\n", dma_phy_addr, ttx_buf);
	}

#if defined(TTX_NO_INTR)
	{
		/* FIXME: SunHe, Temparary Exist Only for test  */
		ttx_setconfig(VBI_TXT_PALB);
		ttx_control(1);

		init_timer(&vbi_timer);
		vbi_timer.function = vbi_ttx_sending;
		mod_timer(&vbi_timer, jiffies + msecs_to_jiffies(1));
	}
#endif

	return 0;
}

inline static void csm_df_remove_dev(dev_t df_dev)
{
    class_simple_device_remove(df_dev);

} 
static void __exit orion15_df_exit(void)
{
	__df_mem_destroy();

	if (NULL != orion15_df_proc_entry)
		remove_proc_entry("df15_io", NULL);

	csm_df_remove_dev(MKDEV(DF_MAJOR, DF_MINOR_GFX0));
	csm_df_remove_dev(MKDEV(DF_MAJOR, DF_MINOR_GFX1));
	csm_df_remove_dev(MKDEV(DF_MAJOR, DF_MINOR_VID0));
	csm_df_remove_dev(MKDEV(DF_MAJOR, DF_MINOR_VID1));
	csm_df_remove_dev(MKDEV(DF_MAJOR, DF_MINOR_OUT0));
	csm_df_remove_dev(MKDEV(DF_MAJOR, DF_MINOR_OUT1));
	csm_df_remove_dev(MKDEV(DF_MAJOR, DF_MINOR_TVE0));
	csm_df_remove_dev(MKDEV(DF_MAJOR, DF_MINOR_TVE1));

    class_simple_destroy(csm_df_class);
	unregister_chrdev(DF_MAJOR, "csm_df");

	printk(KERN_INFO " ORION DF Exit ...... OK. \n");

	return;
}

int Get_Outif_Id(int gfx_id)
{
	return layer_output.gfx_output[gfx_id];
}

#if 1
EXPORT_SYMBOL(Is_TTX);
EXPORT_SYMBOL(Get_Outif_Id);
EXPORT_SYMBOL(DF_Read);
EXPORT_SYMBOL(DF_Write);
EXPORT_SYMBOL(TVE_Read);
EXPORT_SYMBOL(TVE_Write);
EXPORT_SYMBOL(CLK_Read);
EXPORT_SYMBOL(CLK_Write);
EXPORT_SYMBOL(__df_mem_initialize);
EXPORT_SYMBOL(dfreg);
EXPORT_SYMBOL(__df_update_start);
EXPORT_SYMBOL(__df_update_end);
EXPORT_SYMBOL(DFGfxEna);
EXPORT_SYMBOL(__Is_TV);
EXPORT_SYMBOL(DFGfxSetAlpha);
EXPORT_SYMBOL(DFGfxSetColorKey);
EXPORT_SYMBOL(DFGfxColorKeyEna);
EXPORT_SYMBOL(DFSetZOrder);
EXPORT_SYMBOL(DFVideoEna);
#if defined(DEFAULT_SCALER)
EXPORT_SYMBOL(DFVideoSetDefaultCoeff);
#else
EXPORT_SYMBOL(DFSetVideoScalerCfg);
#endif
EXPORT_SYMBOL(DFVideoSetLayerAlpha);
EXPORT_SYMBOL(DFVideoSetSrcCrop);
EXPORT_SYMBOL(DFVideoSetDispWin);
EXPORT_SYMBOL(DFVideoIsDispWinChange);
#endif

module_init(orion15_df_init);
module_exit(orion15_df_exit);

