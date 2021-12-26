#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <asm/semaphore.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

#include <linux/mem_define.h>

#include "tve_1200.h"
#include "orion_wss.h"

MODULE_AUTHOR("Zhongkai Du, <zhongkai.du@celestialsemi.com>");
MODULE_DESCRIPTION("Celestial Semiconductor TVE sub-system driver");
MODULE_LICENSE("GPL");

/* 
 * Registers Map 
 */

#define IDSCS_DAC_CNTL		(0x400 >> 1)	/* index of short array */
#define IDSCS_FREQ_CNTL		(0x108 >> 1)
#define IDSCS_PLL_TVE		(0x40c >> 1)

#define ORION_TVE_BASE		0x10160000
#define ORION_TVE_SIZE		0x00001000

#define ORION_IDSCS_BASE	0x10171000
#define ORION_IDSCS_SIZE	0x00001000

#define TVE_MODE_576I      	0
#define TVE_MODE_480I      	1
#define TVE_MODE_576P      	2
#define TVE_MODE_480P      	3
#define TVE_MODE_720P50    	4
#define TVE_MODE_720P60    	5
#define TVE_MODE_1080I25   	6
#define TVE_MODE_1080I30   	7
#define TVE_MODE_SECAM      	8
#define TVE_MODE_PAL_M		9
#define TVE_MODE_PAL_N		10
#define TVE_MODE_PAL_CN		11
#define TVE_MODE_1080P24    12
#define TVE_MODE_1080P25    13
#define TVE_MODE_1080P30    14

#define TVE_MODE_UNDEF   	-1

#define CSTVE_IOC_MAGIC        'e'

#define CSTVE_IOC_SET_MODE         	 _IOW(CSTVE_IOC_MAGIC, 0x01, int)
#define CSTVE_IOC_GET_MODE         	 _IOW(CSTVE_IOC_MAGIC, 0x02, int)
#define CSTVE_IOC_SET_WHILE_LEVEL	 _IOW(CSTVE_IOC_MAGIC, 0x03, int)
#define CSTVE_IOC_GET_WHILE_LEVEL	 _IOR(CSTVE_IOC_MAGIC, 0x04, int)
#define CSTVE_IOC_SET_BLACK_LEVEL	 _IOW(CSTVE_IOC_MAGIC, 0x05, int)
#define CSTVE_IOC_GET_BLACK_LEVEL	 _IOR(CSTVE_IOC_MAGIC, 0x06, int)
#define CSTVE_IOC_SET_STURATION_LEVEL	 _IOW(CSTVE_IOC_MAGIC, 0x07, int)
#define CSTVE_IOC_GET_STURATION_LEVEL	 _IOR(CSTVE_IOC_MAGIC, 0x08, int)

#define CSTVE_IOC_ENABLE               	_IOW(CSTVE_IOC_MAGIC, 0x0b, int)
#define CSTVE_IOC_DISABLE              	_IOR(CSTVE_IOC_MAGIC, 0x0c, int)

#define CSTVE_IOC_SET_MACROVISION       _IOW(CSTVE_IOC_MAGIC, 0x0d, int)
#define CSTVE_IOC_CVBS_SVIDEO_ENABLE    _IOW(CSTVE_IOC_MAGIC, 0xe, int)


#define CSTVE_IOC_WSS_CTRL	    _IOW(CSTVE_IOC_MAGIC, 0x0f, int)
#define CSTVE_IOC_WSS_SETCONFIG		    _IOW(CSTVE_IOC_MAGIC, 0x10, int)
#define CSTVE_IOC_WSS_SETINFO	    _IOW(CSTVE_IOC_MAGIC, 0x11, int)

#define CSTVE_IOC_TTX_CTRL	    _IOW(CSTVE_IOC_MAGIC, 0x12, int)
#define CSTVE_IOC_TTX_SETCONFIG		    _IOW(CSTVE_IOC_MAGIC, 0x13, int)
#define CSTVE_IOC_TTX_SETINFO	    _IOW(CSTVE_IOC_MAGIC, 0x14, int)

#define CSTVE_IOC_SET_COMP_CHAN    _IOW(CSTVE_IOC_MAGIC, 0x15, int)
#define CSTVE_IOC_SET_DAC0_COMPOSITION    _IOW(CSTVE_IOC_MAGIC, 0x16, int)

static volatile unsigned short *idscs_base = NULL;
static volatile unsigned char  *tve_base = NULL;
static volatile unsigned char  *wss_base = NULL;
static volatile unsigned char  *ttx_base = NULL;
        
static int tve_init_flags = 0; /* uninitialized */
        
DEFINE_SPINLOCK(orion_tve_lock);

#define TTX_ENGINE_BASE	0x41600000
#define TTX_ENG_ENABLE 				0x140
#define TTX_ENG_FM 						0x144

int tve_initialize(void)
{
	volatile unsigned short  *tv656_base = NULL;
	if (tve_init_flags) return 0;
	tv656_base = (unsigned short *)ioremap(0x10171404, 0x10);
	

	if (NULL == tv656_base) 	return -1;
	*tv656_base = (unsigned int)((*tv656_base) | 0x1000);
	iounmap((void *)tv656_base);


	idscs_base = (unsigned short *)ioremap(ORION_IDSCS_BASE, ORION_IDSCS_SIZE);
	if (NULL == idscs_base) 
		return -1;

	tve_base = (unsigned char *)ioremap(ORION_TVE_BASE, ORION_TVE_SIZE);
	if (NULL == tve_base) 
	{
		iounmap((void *)idscs_base);
		return -1;
	}

	wss_base = (unsigned char *)ioremap(ORION_WSS_BASE, ORION_WSS_SIZE);
	if (NULL == wss_base) 
	{
		iounmap((void *)idscs_base);
		iounmap((void *)tve_base);
		return -1;
	}
	
	ttx_base = (unsigned char *)ioremap(TTX_ENGINE_BASE, 0x200);
	if (NULL == ttx_base) 
	{
		iounmap((void *)idscs_base);
		iounmap((void *)tve_base);
		iounmap((void *)wss_base);
		return -1;
	}

	tve_init_flags = 1;

	return 0;
}

int tve_deinitialize(void)
{
	if (tve_init_flags) 
	{
		iounmap((void *)idscs_base);
		iounmap((void *)tve_base);

		tve_init_flags = 0;
	}

	return 0;
}

#if defined(CONFIG_ARCH_ORION_CSM1200) || defined(CONFIG_ARCH_ORION_CSM1201) 

static void __set_disp_clk(unsigned int is_hd, unsigned int is_progressive)
{
	switch((is_hd << 4)| is_progressive) 
	{
		case 0x00:
			idscs_base[IDSCS_FREQ_CNTL] &= (~(0x3 << 4)); /* Disable TVE_CLK6     */
			udelay(1);
			idscs_base[IDSCS_FREQ_CNTL] &= (~(0x1 << 3)); /* Disable DISP/TVE_CLK */
			udelay(1);
			idscs_base[IDSCS_PLL_TVE]    = 0x8618;
			udelay(100);
			idscs_base[IDSCS_FREQ_CNTL] |= ((0x1 << 3));  /* Enable DISP/TVE_CLK  */
			udelay(1);
			idscs_base[IDSCS_FREQ_CNTL] |= ((0x1 << 4));  /* Select TVE_CLK6      */
			udelay(1);

			break;

		case 0x01:
			idscs_base[IDSCS_FREQ_CNTL] &= (~(0x3 << 4)); /* Disable TVE_CLK6     */
			udelay(1);
			idscs_base[IDSCS_FREQ_CNTL] &= (~(0x1 << 3)); /* Disable DISP/TVE_CLK */
			udelay(1);
			idscs_base[IDSCS_PLL_TVE]    = 0x8618;
			udelay(100);
			idscs_base[IDSCS_FREQ_CNTL] |= ((0x1 << 3));  /* Enable DISP/TVE_CLK  */
			udelay(1);
			idscs_base[IDSCS_FREQ_CNTL] |= ((0x2 << 4));  /* Select TVE_CLK6      */
			udelay(1);

			break;

		case 0x10:
		case 0x11:
			idscs_base[IDSCS_FREQ_CNTL] &= (~(0x3 << 4)); /* Disable TVE_CLK6     */
			udelay(1);
			idscs_base[IDSCS_FREQ_CNTL] &= (~(0x1 << 3)); /* Disable DISP/TVE_CLK */
			udelay(1);
			idscs_base[IDSCS_PLL_TVE]    = 0x4621;
			udelay(100);
			idscs_base[IDSCS_FREQ_CNTL] |= ((0x1 << 3));  /* Enable DISP/TVE_CLK  */
			udelay(1);
			idscs_base[IDSCS_FREQ_CNTL] |= ((0x1 << 4));  /* Select TVE_CLK6      */
			udelay(1);

			break;
	}	
}

#elif defined(CONFIG_ARCH_ORION_CSM1100)

static void __set_disp_clk(unsigned int is_hd, unsigned int is_progressive)
{
	switch((is_hd << 8)| is_progressive) 
	{
		case 0x00:
			idscs_base[IDSCS_FREQ_CNTL] = (idscs_base[IDSCS_FREQ_CNTL] & ~0x0f) | 0x05;
			idscs_base[IDSCS_FREQ_CNTL] = (idscs_base[IDSCS_FREQ_CNTL] & ~0x30) | 0x10;
			break;

		case 0x01:
			idscs_base[IDSCS_FREQ_CNTL] = (idscs_base[IDSCS_FREQ_CNTL] & ~0x0f) | 0x05;
			idscs_base[IDSCS_FREQ_CNTL] = (idscs_base[IDSCS_FREQ_CNTL] & ~0x30) | 0x20;
			break;

		case 0x10:
		case 0x11:
			idscs_base[IDSCS_FREQ_CNTL] = (idscs_base[IDSCS_FREQ_CNTL] & ~0x0f) | 0x05;
			idscs_base[IDSCS_FREQ_CNTL] = (idscs_base[IDSCS_FREQ_CNTL] & ~0x30) | 0x10;
			break;
	}
}

#endif

extern int video_disp_out_fmt(int layer, unsigned int fmt); /* Zhongkai's ugly code */
extern int __df_disp_enable(int en_flags);/*majia add here*/

static unsigned int old_fmt = TVE_MODE_UNDEF;

int get_tve_output(void)
{
	return old_fmt;
}

int set_tve_output(unsigned int fmt)
{
	unsigned int i;

	if (fmt == old_fmt) return 0;

	__df_disp_enable(0);
	video_disp_out_fmt(0, fmt);	/* setting display format */
	
	switch(fmt) 
	{
		case TVE_MODE_PAL_M:
			__set_disp_clk(0, 0);


			tve_base[0x0] = 0x1;
			udelay(1);

			for(i = 0; i < sizeof(tve_regs)/sizeof(struct tve_reg); i++) 
			{
				tve_base[tve_regs[i].reg_addr] = tve_regs[i].palm_val;
			}

			tve_base[0x0] = 0x0;

			break;
			
		case TVE_MODE_PAL_N:
			__set_disp_clk(0, 0);

			tve_base[0x0] = 0x1;
			udelay(1);

			for(i = 0; i < sizeof(tve_regs)/sizeof(struct tve_reg); i++) 
			{
				tve_base[tve_regs[i].reg_addr] = tve_regs[i].paln_val;
			}

			tve_base[0x0] = 0x0;

			break;
			
		case TVE_MODE_PAL_CN:
			__set_disp_clk(0, 0);

			tve_base[0x0] = 0x1;
			udelay(1);

			for(i = 0; i < sizeof(tve_regs)/sizeof(struct tve_reg); i++) 
			{
				tve_base[tve_regs[i].reg_addr] = tve_regs[i].palcn_val;
			}

			tve_base[0x0] = 0x0;

			break;

		case TVE_MODE_SECAM:
			__set_disp_clk(0, 0);

			tve_base[0x0] = 0x1;
			udelay(1);

			for(i = 0; i < sizeof(tve_regs)/sizeof(struct tve_reg); i++) 
			{
				tve_base[tve_regs[i].reg_addr] = tve_regs[i].secam_val;
			}

			tve_base[0x0] = 0x0;

			break;
			
		case TVE_MODE_576I:
			__set_disp_clk(0, 0);

			tve_base[0x0] = 0x1;
			udelay(1);

			for(i = 0; i < sizeof(tve_regs)/sizeof(struct tve_reg); i++) 
			{
				tve_base[tve_regs[i].reg_addr] = tve_regs[i].pal_val;
			}

			tve_base[0x0] = 0x0;

			break;

		case TVE_MODE_480I:
			__set_disp_clk(0, 0);

			tve_base[0x0] = 0x1;
			udelay(1);

			for(i = 0; i < sizeof(tve_regs)/sizeof(struct tve_reg); i++) 
			{
				tve_base[tve_regs[i].reg_addr] = tve_regs[i].ntsc_val;
			}

			tve_base[0x0] = 0x0;

			break;

		case TVE_MODE_720P60:
		case TVE_MODE_720P50:
			__set_disp_clk(1, 1);

			tve_base[0x0] = 0x1;
			udelay(1);

			for(i = 0; i < sizeof(tve_regs)/sizeof(struct tve_reg); i++) 
			{
				tve_base[tve_regs[i].reg_addr] = tve_regs[i].p720_val;
			}

			tve_base[0x0] = 0x0;

			break;

		case TVE_MODE_1080I30:
		case TVE_MODE_1080I25:
			__set_disp_clk(1, 0);

			tve_base[0x0] = 0x1;
			udelay(1);

			for(i = 0; i < sizeof(tve_regs)/sizeof(struct tve_reg); i++) 
			{
				tve_base[tve_regs[i].reg_addr] = tve_regs[i].i1080_val;
			}

			tve_base[0x0] = 0x0;

			break;

		case TVE_MODE_480P:
		case TVE_MODE_576P:
			__set_disp_clk(0, 1);

			tve_base[0x0] = 0x1;
			udelay(1);

			for(i = 0; i < sizeof(tve_regs)/sizeof(struct tve_reg); i++) 
			{
				tve_base[tve_regs[i].reg_addr] = tve_regs[i].p480_val; 
			}

			tve_base[0x0] = 0x0;

			break;

    case TVE_MODE_1080P24:
    case TVE_MODE_1080P25:
    case TVE_MODE_1080P30:
        __set_disp_clk(1, 1);
        
        break;

		default:
			return -ENXIO;
	}

	__df_disp_enable(1);
	old_fmt = fmt;
	
	return 0;
}

/*=============================== CONFIG_WSS =====================================*/

//static u32 DevBaseAddr = TVE_BASE_ADDR;
#define DevBaseAddr		(wss_base)


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
	if(Enable == 1)
	{
		ValCfg |= 0x3;
	}
	else
	{
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
	//u8 Hichar=0,Lochar=0;


	if(WssStandard == VBI_WSS_PAL)
	{		
		ValCfg = 0x64;
	
		ValClk = 0x17b;
		ValLevel = 642;
		LineNum = 22;
	}
	else
	{
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
	//u8 Hichar=0,Lochar=0;

	TVE_VsyncType_t FieldType;

	FieldType = VbiData_p->FieldType;

	switch(VbiData_p->WssType)
	{
		case TVE_VBI_WSS:
			{
				u16 *Data_p;
				Data_p = (u16 *)VbiData_p->VbiData.Buf;

				if(FieldType == TVE_FIELD_TOP)				
				{
					TVE_WriteRegDev16BE(DevBaseAddr, TVE_WSS_DATAF0_BASE+1, *Data_p);	
					//Lochar=*Data_p;
					//Hichar=((*Data_p)>>8);
					//wss_base[TVE_WSS_DATAF0_BASE+1]=Lochar;
					//wss_base[TVE_WSS_DATAF0_BASE+2]=Hichar;
				}	
				else														
				{
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

				if(FieldType == TVE_FIELD_TOP)				
				{
					TVE_WriteRegDev20BE(DevBaseAddr, TVE_WSS_DATAF0_BASE, *Data_p);
					//Lochar=*Data_p;
					//Hichar=((*Data_p)>>8);
					//wss_base[TVE_WSS_DATAF0_BASE]=Lochar;
					//wss_base[TVE_WSS_DATAF0_BASE+1]=Hichar;
				}	
				else																
				{
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

	switch(WssInfo_p->ARatio)
	{
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
	if(error_code != 0)	
	{
		printk(" error VOS_WriteVbiData\n");
	}	

	VbiData.FieldType = TVE_FIELD_BOT;	
	VbiData.VbiData.Buf = DataBuf;
	error_code = WSS_WriteData( &VbiData);
	if(error_code != 0)	
	{
		printk(" error VOS_WriteVbiData\n");
	}	

	return error_code;

}





/*-------------------------------------------------------------------------
 * Function : TVE_SetVbiConfig
 *	      Set the parameters
 * Input    : 
 * Output   :
 * Return   : Error Code
 * ----------------------------------------------------------------------*/
int TTX_Ctrl(u8 Enable)
{
	int	   Error = 0;

	u8 ValCfg = 0;

	TVE_WriteRegDev32(ttx_base, TTX_ENG_ENABLE, 0);		/* disable : mips of video */

	//ValCfg = wss_readb(TVE_TTX_CONFIG);
	ValCfg=wss_base[TVE_TTX_CONFIG];

	if(Enable == 1)
	{
		ValCfg |= 0x1;
	}
	else
	{
		ValCfg &= ~0x1;
	}

	TVE_WriteRegDev8(DevBaseAddr, TVE_TTX_CONFIG, ValCfg);		/* ttx encoder */


	return ( Error );
}


/*-------------------------------------------------------------------------
 * Function : TVE_SetVbiConfig
 *	      Set the parameters
 * Input    : 
 * Output   :
 * Return   : Error Code
 * ----------------------------------------------------------------------*/
int TTX_SetConfig(VBI_TxtStandard_t TxtStandard)
{
	int	   Error = 0;

	u8 ValCfg = 0;
	
	u16 ValClk = 0;
	u16 ValLevel = 0;
	u8 LineNum0 = 0;
	u8 LineNum1 = 0;

	u8 HStart;
	u8 HLength;

	ValCfg = 0x1c;

	if(TxtStandard == VBI_TXT_PALB)
	{		
		ValClk = 0x41c;
		ValLevel = 613;
		LineNum0 = 5;
		LineNum1 = 21;
		HStart = 72;
		HLength = 42;
	}
	else
	{
		ValClk = 0x365;
		ValLevel = 635;	
		LineNum0 = 9;
		LineNum1 = 20;
		HStart = 55;
		HLength = 34;
	}

	TVE_WriteRegDev8(DevBaseAddr, TVE_TTX_CONFIG, ValCfg);

	TVE_WriteRegDev8(DevBaseAddr, TVE_TTX_STARTPOS, HStart);
	TVE_WriteRegDev8(DevBaseAddr, TVE_TTX_DATALENGTH	, HLength);
	TVE_WriteRegDev8(DevBaseAddr, TVE_TTX_FCODE, 0x27);

	TVE_WriteRegDev12(DevBaseAddr, TVE_TTX_CLOCK_BASE, ValClk);
	TVE_WriteRegDev10(DevBaseAddr, TVE_TTX_LEVEL_BASE, ValLevel);
	TVE_WriteRegDev10(DevBaseAddr, TVE_TTX_LINEF0START_BASE, LineNum0);
	TVE_WriteRegDev10(DevBaseAddr, TVE_TTX_LINEF1START_BASE, LineNum0);
	TVE_WriteRegDev10(DevBaseAddr, TVE_TTX_LINEF0END_BASE, LineNum1);	
	TVE_WriteRegDev10(DevBaseAddr, TVE_TTX_LINEF1END_BASE, LineNum1);
        
	return ( Error );
}





#define TVE_TXT_LINEDISABLE_BASE 		0x89

#define TVE_TTC_ENA 				0x100		
#define TVE_TTC_FW_ADDR 		0x104		


static void VBI_Delay(int ms)
{int i,j=0;
	for(i=0;i<ms;i++)
		for(j=0;j<100000;j++);
}

/*-------------------------------------------------------------------------
 * Function : TTVO_WriteCcData
 * Input    : 
 * Output   :
 * Return   : TRUE if error, FALSE if success
 * ----------------------------------------------------------------------*/
int VBI_SetTxtPage( TTX_Page_t  *Page_p )
{

         int error_code = 0;

 

         TTX_Page_t TtxBuf;

 

    u32 FieldLines = 0;

         u16 BitMask = 0xffff;

         int i;

        u8  read_value;

 

         copy_from_user(&TtxBuf, Page_p,sizeof(TTX_Page_t));

 

   FieldLines = TtxBuf.ValidLines;

    for(i=0x79;i<0x8b;i++)
        {
        read_value = wss_base[i] ;
        printk(KERN_INFO "TVE_reg[0x%x] = 0x%x\n",i, read_value);
        }    
 

         TVE_WriteRegDev32(DevBaseAddr, TVE_TTC_ENA, 0);

 

         TVE_WriteRegDev16(DevBaseAddr, TVE_TXT_LINEDISABLE_BASE, 0xffff);

 

         //

         TVE_WriteRegDev32(DevBaseAddr, TVE_TTC_FW_ADDR, (u32)TtxBuf.Lines);                // ttx data buffer
	printk(KERN_INFO "line[0]: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x...\n",TtxBuf.Lines[0][0],
	TtxBuf.Lines[0][1],TtxBuf.Lines[0][2],TtxBuf.Lines[0][3],TtxBuf.Lines[0][4]);
 

         for(i = 0; i<FieldLines; i++)

         {

                   BitMask &= ~(1<<i);

         }

         TVE_WriteRegDev16(DevBaseAddr, TVE_TXT_LINEDISABLE_BASE, BitMask);

 

 

         TVE_WriteRegDev32(DevBaseAddr, TVE_TTC_ENA, 1);           /* engine */

         

         return error_code;

 

}

/*-------------------------------------------------------------------------
 * Function : VOUT_SetCompChan
 *	      Set the parameters
 * Input    : 
 * Output   :
 * Return   : Error Code
 * ----------------------------------------------------------------------*/
int VOUT_SetCompChan( CSVOUT_CompChannType_t CompChan)
{
	int	   Error = 0;

	u8 ValOutputCtrl= 0;

	//ValOutputCtrl = wss_readb(TVE_OUTPUT_CONFIG);
	ValOutputCtrl=wss_base[TVE_OUTPUT_CONFIG];

	if(CompChan == CSVOUT_COMP_YVU)
	{
		ValOutputCtrl |= 0x8;
	}
	else
	{
		ValOutputCtrl &= ~0x8;
	}

	TVE_WriteRegDev8(DevBaseAddr, TVE_OUTPUT_CONFIG, ValOutputCtrl);

	return ( Error );
}




/*=============================== End CONFIG_WSS =====================================*/

static int orion_tve_ioctl(struct inode *inode, struct file *file,
			   unsigned int cmd, unsigned long arg)
{
	unsigned long flags;
	int      retval = -1;
	unsigned int reg_val;

	switch (cmd) {

		case CSTVE_IOC_SET_MODE:
			spin_lock_irqsave(&orion_tve_lock, flags);
			retval = set_tve_output(arg);	/* setting TVE output format */
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			break;

		case CSTVE_IOC_GET_MODE:
			spin_lock_irqsave(&orion_tve_lock, flags);
			reg_val = get_tve_output();	/* setting TVE output format */
			__put_user(reg_val, (int __user *) arg); 
			if(reg_val != TVE_MODE_UNDEF)  retval =0;
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			break;

		case CSTVE_IOC_SET_WHILE_LEVEL:
			spin_lock_irqsave(&orion_tve_lock, flags);
			tve_base[0x27] =  (arg>>8)&0x03;
			tve_base[0x28] =  arg&0xff;
			retval = 0;
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			break;

		case CSTVE_IOC_GET_WHILE_LEVEL:
			spin_lock_irqsave(&orion_tve_lock, flags);
			reg_val = tve_base[0x27];
			reg_val <<=8;
			reg_val |= tve_base[0x28];
			reg_val &=0x3ff; 
			retval = 0;
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			__put_user(reg_val, (int __user *) arg); 
			break;

		case CSTVE_IOC_SET_BLACK_LEVEL:
			spin_lock_irqsave(&orion_tve_lock, flags);
			tve_base[0x29] =  (arg>>8)&0x03;
			tve_base[0x2a] =  arg&0xff;
			retval = 0;
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			break;

		case CSTVE_IOC_GET_BLACK_LEVEL:
			spin_lock_irqsave(&orion_tve_lock, flags);
			reg_val = tve_base[0x29];
			reg_val <<=8;
			reg_val |= tve_base[0x2a];
			reg_val &=0x3ff; 
			retval = 0;
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			__put_user(reg_val, (int __user *) arg); 
			break;

		case CSTVE_IOC_SET_STURATION_LEVEL:
			spin_lock_irqsave(&orion_tve_lock, flags);
			tve_base[0x23] =  arg&0xff;
			tve_base[0x24] =  arg&0xff;
			retval = 0;
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			break;

		case CSTVE_IOC_GET_STURATION_LEVEL:
			spin_lock_irqsave(&orion_tve_lock, flags);
			reg_val = tve_base[0x23];
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			__put_user(reg_val, (int __user *) arg); 
			break;

		case CSTVE_IOC_ENABLE:
			reg_val = old_fmt;
			old_fmt = -1;
			set_tve_output(reg_val);		
			break;

		case CSTVE_IOC_DISABLE:
			idscs_base[IDSCS_FREQ_CNTL] &= (~0x8); /* Disable TVE_CLK. */
			break;

		case CSTVE_IOC_SET_MACROVISION:
			spin_lock_irqsave(&orion_tve_lock, flags);
			if (arg > 0){
				tve_base[0x40] = 0x3f;
				tve_base[0x59] = 0;
			}else{
				tve_base[0x40] = 0;
				tve_base[0x59] = 0;
			}
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			break;

		case CSTVE_IOC_WSS_CTRL:
			spin_lock_irqsave(&orion_tve_lock, flags);
			WSS_Ctrl((VBI_WssStandard_t)arg);
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;

		case CSTVE_IOC_WSS_SETCONFIG:
			spin_lock_irqsave(&orion_tve_lock, flags);
			WSS_SetConfig((VBI_WssStandard_t)arg);
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;
		case CSTVE_IOC_WSS_SETINFO:
			spin_lock_irqsave(&orion_tve_lock, flags);
			VBI_SetWssInfo((VBI_WssInfo_t *)arg);
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;
			
		case CSTVE_IOC_TTX_CTRL:
			spin_lock_irqsave(&orion_tve_lock, flags);
			TTX_Ctrl((CSVOUT_CompChannType_t)arg);
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;

		case CSTVE_IOC_TTX_SETCONFIG:
			spin_lock_irqsave(&orion_tve_lock, flags);
			TTX_SetConfig((CSVOUT_CompChannType_t)arg);
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;

		case CSTVE_IOC_TTX_SETINFO:
			spin_lock_irqsave(&orion_tve_lock, flags);
			VBI_SetTxtPage((TTX_Page_t *)arg);
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;

		case CSTVE_IOC_SET_COMP_CHAN:
			spin_lock_irqsave(&orion_tve_lock, flags);
			VOUT_SetCompChan((CSVOUT_CompChannType_t)arg);
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;

		case CSTVE_IOC_SET_DAC0_COMPOSITION:
			spin_lock_irqsave(&orion_tve_lock, flags);

			if(arg == 1)
				idscs_base[IDSCS_DAC_CNTL] = (idscs_base[IDSCS_DAC_CNTL] & 0xfffe);
			else
				idscs_base[IDSCS_DAC_CNTL] = (idscs_base[IDSCS_DAC_CNTL] |0x1);

			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;
			break;

		case CSTVE_IOC_CVBS_SVIDEO_ENABLE:
			spin_lock_irqsave(&orion_tve_lock, flags);
			if (arg > 0){
				reg_val = tve_base[0x4];
				tve_base[0x4] = reg_val | 0x1;
			}else{
				reg_val = tve_base[0x4];
				tve_base[0x4] = reg_val & (~0x1);
			}
			spin_unlock_irqrestore(&orion_tve_lock, flags);
			retval = 0;	
			break;

			
		default: 
			retval = -1;
	}

	return retval;
}

static struct file_operations orion_tve_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= orion_tve_ioctl,
};

static struct miscdevice orion_tve_miscdev = {
	MISC_DYNAMIC_MINOR,
	"orion_tve",
	&orion_tve_fops
};

static struct proc_dir_entry *tve_proc_entry = NULL;

static int tve_proc_write(struct file *file, 
			  const char *buffer, unsigned long count, void *data)
{
	u32 addr, val;
	const char *cmd_line = buffer;;

	if (strncmp("init", cmd_line, 2) == 0) 
	{
		set_tve_output(0);
	}
	else if (strncmp("rb", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = tve_base[addr];
		printk(" 0x%02x = 0x%02x \n", addr, val);
	}
	else if (strncmp("wb", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = simple_strtol(&cmd_line[6], NULL, 16);
		tve_base[addr] = val;
	}
	else if (strncmp("rw", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = idscs_base[addr>>1];
		printk(" readw [0x%02x] = 0x%02x \n", addr, val);
	}
	else if (strncmp("ww", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = simple_strtol(&cmd_line[7], NULL, 16);
		idscs_base[addr>>1] = val;
	}

	return count;
}

int __init orion_tve_init(void)
{
	int ret = 0;

	ret = -ENODEV;
	if (misc_register(&orion_tve_miscdev)) 
		goto ERR_NODEV;

	ret = -EIO;
	if (0 != tve_initialize()) goto ERR_INIT;

	set_tve_output(0); /* to set a default TV mode, 576I */

	tve_proc_entry = create_proc_entry("tve_io", 0, NULL);
	if (NULL != tve_proc_entry) {
		tve_proc_entry->write_proc = &tve_proc_write;
	}

	printk(KERN_INFO "%s: Orion TVE driver was initialized, at address@[phyical addr = %08x, size = %x] \n", 
	       "orion_tve", ORION_TVE_BASE, ORION_TVE_SIZE);

	return 0;

ERR_INIT:
ERR_NODEV:
	return ret;
}	

static void __exit orion_tve_exit(void)
{
	tve_deinitialize();
	misc_deregister(&orion_tve_miscdev);
}

module_init(orion_tve_init);
module_exit(orion_tve_exit);

EXPORT_SYMBOL(tve_initialize);
EXPORT_SYMBOL(tve_deinitialize);
EXPORT_SYMBOL(set_tve_output);

