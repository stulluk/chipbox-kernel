#ifndef _TVE0_1201_H
#define _TVE0_1201_H

//2 TVE
#define  TVE0_REG_NUM		(45)
#define  REG_ADDR    		(0) 
#define  TYPE_NTSC   		(1)
#define  TYPE_PAL    		(2)
#define  TYPE_PAL_M  		(3)
#define  TYPE_PAL_N  		(4)
#define  TYPE_PAL_CN 		(5)
#define  TYPE_SECAM  		(6)
#define  TYPE_480p   		(7)
#define  TYPE_720p50   		(8)
#define  TYPE_720p60   		(9)
#define  TYPE_1080i  		(10)
#define  TYPE_VGA		(11)//duliguo
#define  TVE0_TYPE_TOTAL  	(12)
#define  TYPE_576p   		(10) //1440*576
#define  TYPE_576i   		(11)   
#define  TYPE_480i   		(12) 

//1TVE0

//dont change this table
static unsigned char TVE0_REG[TVE0_REG_NUM][TVE0_TYPE_TOTAL] =
{
	   /*   address	ntsc	pal	PAL_M 	PAL-N	PAL-CN	SECAM	480p	720p50	720p60	1080i	VGA	*/
	/*0*/	{0x0,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},	//CVE_SOFT_RESET_VAL
	/*1*/	{0x1,	0x50,	0x50,	0x50,	0x50,	0x50,	0x50,	0x50,	0x50,	0x50,	0x50,	0x50},	//CVE_REV_ID_VAL
	/*2*/	{0x2,	0x1d,	0x1d,	0x1d,	0x1d,	0x1d,	0x1d,	0x1d,	0x1d,	0x1d,	0x1d,	0x1d},	//CVE_INPUT_CTL_VAL
	/*3*/	{0x4,	0x09,	0x09,	0x09,	0x09,	0x09,	0x09,	0x09,	0x09,	0x09,	0x09,	0x09},	//CVE_OUT_CTL_VAL
	/*4*/	{0x6,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01},	//CVE_SLV_MD_TH_VAL
	/*5*/	{0x7,	0x00,	0x04,	0x24,	0x24,	0x34,	0x3C,	0x01,	0x03,	0x03,	0x02,	0x01},	//CVE_VID_STD_VAL
	/*6*/	{0x8,	0x02,	0x02,	0x02,	0x02,	0x02,	0x02,	0x02,	0x02,	0x02,	0x04,	0x02},	//CVE_NUM_LINES_H_VAL
	/*7*/	{0x9,	0x0d,	0x71,	0x0D,	0x71,	0x71,	0x71,	0x0D,	0xEE,	0xEE,	0x65,	0x0D},	//CVE_NUM_LINES_L_VAL
	/*8*/	{0xa,	0x15,	0x17,	0x15,	0x17,	0x17,	0x17,	0x2a,	0x19,	0x19,	0x14,	0x2c},	//CVE_FST_VID_LINE_VAL
	/*9*/	{0xb,	0x7e,	0x7e,	0x7e,	0x7e,	0x7e,	0x7e,	0x7e,	0x40,	0x50,	0x66,	0xC0},	//CVE_HSYNC_WIDTH_VAL
	/*10*/	{0xd,	0x68,	0x80,	0x76,	0x8a,	0x8a,	0x8a,	0x68,	0xdc,	0xd0,	0x80,	0x60},	//CVE_BACK_PORCH_VAL
	/*11*/	{0xe,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x01,	0x00,	0x00,	0x00},	//CVE_FRONT_PORCH_H_VAL
	/*12*/	{0xf,	0x32,	0x28,	0x20,	0x18,	0x18,	0x18,	0x32,	0xb8,	0x48,	0x40,	0x20},	//CVE_FRONT_PORCH_L_VAL
	/*13*/	{0x10,	0x05,	0x05,	0x05,	0x05,	0x05,	0x05,	0x05,	0x05,	0x05,	0x07,	0x05},	//CVE_ACTIVELINE_H_VAL
	/*14*/	{0x11,	0x9c,	0x9a,	0xa0,	0xa0,	0xa0,	0xa0,	0x9c,	0x00,	0x0a,	0x80,	0x00},	//CVE_ACTIVELINE_L_VAL
	/*15*/	{0x12,	0x44,	0x40,	0x44,	0x40,	0x44,	0x40,	0x44,	0x08,	0x08,	0x08,	0x44},	//CVE_BURST_WDITH_VAL
	/*16*/	{0x13,	0x16,	0x1a,	0x12,	0x1a,	0x1a,	0x1a,	0x16,	0x08,	0x08,	0x08,	0x16},	//CVE_BREEZE_WAY_VAL
	/*17*/	{0x16,	0x10,	0x15,	0x10,	0x15,	0x10,	0x14,	0x15,	0x15,	0x15,	0x15,	0x15},	//CVE_CHROMA_FREQ_3_VAL
	/*18*/	{0x17,	0xf8,	0x04,	0xF3,	0x04,	0xFB,	0xE3,	0x04,	0x04,	0x04,	0x04,	0x04},	//CVE_CHROMA_FREQ_2_VAL
	/*19*/	{0x18,	0x3e,	0xc5,	0x77,	0xc5,	0x4A,	0x8E,	0xc5,	0xc5,	0xc5,	0xc5,	0xC5},	//CVE_CHROMA_FREQ_1_VAL
	/*20*/	{0x19,	0x0f,	0x66,	0xD2,	0x66,	0x23,	0x39,	0x66,	0x66,	0x66,	0x66,	0x66},	//CVE_CHROMA_FREQ_0_VAL
	/*21*/	{0x1a,	0x14,	0x14,	0x14,	0x14,	0x00,	0x14,	0x14,	0x14,	0x14,	0x14,	0x14},	/*freq2*/
	/*22*/	{0x1b,	0x25,	0x25,	0x25,	0x00,	0x00,	0x25,	0x25,	0x25,	0x25,	0x25,	0x25},	
	/*23*/	{0x1c,	0xed,	0xed,	0xed,	0x00,	0x00,	0xed,	0xed,	0xed,	0xed,	0xed,	0xED},	
	/*24*/	{0x1d,	0x09,	0x09,	0x09,	0x00,	0x00,	0x09,	0x09,	0x09,	0x09,	0x09,	0x09},	
	/*25*/	{0x1e,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},	//CVE_CHROMA_PHASE_VAL
	/*26*/	{0x1f,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},	//CVE_TINT_VAL
	/*27*/	{0x20,	0x14,	0x04,	0x04,	0x04,	0x04,	0x04,	0x04,	0x04,	0x04,	0x04,	0x04},	//CVE_NA_WI_CH_CH_CP_VAL
	/*28*/	{0x21,	0x44,	0x30,	0x2a,	0x2a,	0x2c,	0x00,	0x30,	0x30,	0x30,	0x30,	0x30},	//CVE_CB_BURST_AMP_VAL
	/*29*/	{0x22,	0x00,	0x24,	0x1d,	0x1d,	0x1f,	0x00,	0x24,	0x24,	0x24,	0x24,	0x24},	//CVE_CR_BURST_AMP_VAL
	/*30*/	{0x23,	0xa2,	0xb2,	0xa2,	0xa2,	0xa2,	0xa2,	0xb8,	0xb8,	0xb8,	0xb8,	0xD4},	//CVE_CB_GAIN_VAL
	/*31*/	{0x24,	0xa2,	0xb2,	0xa2,	0xa2,	0xa2,	0xa2,	0xb8,	0xb8,	0xb8,	0xb8,	0xD0},	//CVE_CR_GAIN_VAL
	/*32*/	{0x27,	0x02,	0x03,	0x02,	0x02,	0x02,	0x02,	0x02,	0x02,	0x02,	0x02,	0x03},	//CVE_WHITE_LEVEL_H_VAL
	/*33*/	{0x28,	0xf8,	0x10,	0xf8,	0x00,	0xf8,	0xf8,	0xf8,	0xf8,	0xf8,	0xf8,	0x28},	//CVE_WHITE_LEVEL_L_VAL
                                                                                        
	/*34*/	{0x29,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},	//CVE_BLACK_LEVEL_H_VAL
	/*35*/	{0x2a,	0xb0,	0xb0,	0xb0,	0xb0,	0xb0,	0xb0,	0xb0,	0xfb,	0xfb,	0xb0,	0xB0},	//CVE_BLACK_LEVEL_L_VAL
                                                                                        
	/*36*/	{0x2b,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01},	//CVE_BLANK_LEVEL_H_VAL
	/*37*/	{0x2c,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},	//CVE_BLANK_LEVEL_L_VAL
                                                                                        
	/*38*/	{0x2d,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01},	//CVE_VBI_BLANK_LEVEL_H_VAL
	/*39*/	{0x2e,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},	//CVE_VBI_BLANK_LEVEL_L_VAL
                                                                                        
	/*40*/	{0x2f,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01},	//CVE_CLAMP_LEVEL_H_VAL
	/*41*/	{0x30,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},	//CVE_CLAMP_LEVEL_L_VAL
                                                                                        
	/*42*/	{0x31,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10},	//CVE_SYNC_LEVEL_VAL
	/*43*/	{0x33,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01,	0x01},	//CVE_SYNC_HIGH_LEVEL_H_VAL
	/*44*/	{0x34,	0xeb,	0xeb,	0xeb,	0xeb,	0xeb,	0xeb,	0xeb,	0xeb,	0xeb,	0xeb,	0xEB},	//CVE_SYNC_HIGH_LEVEL_L_VAL
	//*45*/	{0x37,	0x09,	0x15,	0x08,	0x08,	0x08,	0x08,	0x15,	0x15,	0x15,	0x15,	0x15},	//CVE_NOTCH_EN_WI_FR_VAL
	//*46*/	{0x38,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},	//CVE_NO_SH_NO_VAL
	//*47*/	{0x39,	0x05,	0x05,	0x05,	0x05,	0x05,	0x05,	0x05,	0x05,	0x05,	0x05,	0x05},	//CVE_NOISE_THRESHOLD_VAL
	//*48*/	{0x3a,	0x0a,	0x0a,	0x0a,	0x0a,	0x0a,	0x0a,	0x0a,	0x0a,	0x0a,	0x0a,	0x0A},	//CVE_SHARPEN_THRESHOLD_VAL
};

#endif
