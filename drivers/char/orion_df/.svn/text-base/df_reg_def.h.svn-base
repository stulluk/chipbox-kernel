#ifndef _DF_REG_DEF_H_
#define _DF_REG_DEF_H_

/****************Register IF******************************/

#define TVE0_REG_BASE		0x10168000
#define TVE0_REG_SIZE		0x00001000
#define TVE1_REG_BASE		0x10160000
#define TVE1_REG_SIZE		0x00001000

enum _CLK_REGISTER_BANK_
{
	REG_CLK_BASE = 0x10171000,
	REG_CLK_IDS_ID_LO   = REG_CLK_BASE + 0x000,
	REG_CLK_IDS_ID_HI   = REG_CLK_BASE + 0x004,
	REG_CLK_PLL_DDR_DIV = REG_CLK_BASE + 0x100,
	REG_CLK_PLL_BP      = REG_CLK_BASE + 0x104,
	REG_CLK_FREQ_CTRL   = REG_CLK_BASE + 0x108,
	REG_CLK_AUD_CLK_GEN_HPD    = REG_CLK_BASE + 0x10c,
	REG_CLK_AUD_CLK_GEN_FREQ   = REG_CLK_BASE + 0x118,
	REG_CLK_AUD_CLK_GEN_JITTER = REG_CLK_BASE + 0x11c,
	REG_CLK_SOFT_RESET  = REG_CLK_BASE + 0x200,
	REG_CLK_OPROC_CTRL  = REG_CLK_BASE + 0x400,

	// ----------Test set registers ----------
	IDS_MBIST_DONE      = REG_CLK_BASE +  0x304,
	IDS_MBIST_RESULT0   = REG_CLK_BASE +  0x308,
	IDS_MBIST_RESULT1   = REG_CLK_BASE +  0x30c,
	IDS_TEST_CTRL       = REG_CLK_BASE +  0x310,

	// ----------System control register ----------
	IDS_OPROC_LOCTRL    = REG_CLK_BASE +  0x400,//pin mux0
	IDS_OPROC_HICTRL    = REG_CLK_BASE +  0x404,//pin mux1
	IDS_PCTL_HI         = REG_CLK_BASE +  0x408,
	IDS_PCTL_LO         = REG_CLK_BASE +  0x40c,

	//AHB error capturer
	IDS_HER_IND         = REG_CLK_BASE +  0x410,
	IDS_HER0_STAT0      = REG_CLK_BASE +  0x414,
	IDS_HER0_STAT1      = REG_CLK_BASE +  0x418,
	IDS_HER1_STAT0      = REG_CLK_BASE +  0x41c,
	IDS_HER1_STAT1      = REG_CLK_BASE +  0x420,
	IDS_HER2_STAT0      = REG_CLK_BASE +  0x424,
	IDS_HER2_STAT1      = REG_CLK_BASE +  0x428,

	REG_CLK_SIZE = 0x1000,
};

enum  _DF_REGISTER_BANK_
{
	DISP_REG_BASE             = 0x41800000,	
	DISP_REG_COMMON_BASE      = DISP_REG_BASE + (0x00 << 2),//0x0
	DISP_REG_GFX1_BASE        = DISP_REG_BASE + (0x10 << 2),//0x40
	DISP_REG_GFX2_BASE        = DISP_REG_BASE + (0x20 << 2),//0x80
	DISP_REG_VIDEO1_BASE      = DISP_REG_BASE + (0x30 << 2),//0xc0
	DISP_REG_VIDEO2_BASE      = DISP_REG_BASE + (0x50 << 2),//0x140
	DISP_REG_COMP1_BASE       = DISP_REG_BASE + (0x70 << 2),//0x1c0
	DISP_REG_COMP2_BASE       = DISP_REG_BASE + (0x80 << 2),//0x200
	DISP_REG_HD2SD_BASE       = DISP_REG_BASE + (0x90 << 2),//0x240
	DISP_REG_OUTIF1_BASE      = DISP_REG_BASE + (0xA0 << 2),//0x280
	DISP_REG_OUTIF2_BASE      = DISP_REG_BASE + (0xB0 << 2),//0x2c0
	
	/***************COMMON Register******************/
	DISP_UPDATE_REG           = ( DISP_REG_COMMON_BASE ) + ( 0 << 2),
	/*
	// Bit[   0] : Write 1 Before Update Any Register
	//             Write 0 After Finish Update Register
	*/
	DISP_STATUS               = ( DISP_REG_COMMON_BASE ) + ( 1 << 2),
	/* Readonly Status Register, donot need doubled
	// Bit[    0] : OutputIF1 Error, Reset Value 0;
	// Bit[7 : 1] : OutputIF1 Error Type, Reset Value 0;
	// Bit[    8] : OutputIF2 Error, Reset Value 0;
	// Bit[15: 9] : OutputIF2 Error Type, Reset Value 0;
	// Bit[   16] : OutputIF1 Frame Sync Interrupt
	// Bit[   17] : OutputIF2 Frame Sync Interrupt
	//       *Note:
	*/
	DISP_OUTIF1_INT_CLEAR     = ( DISP_REG_COMMON_BASE ) + ( 2 << 2),
	DISP_OUTIF2_INT_CLEAR     = ( DISP_REG_COMMON_BASE ) + ( 3 << 2),
	/*
 	// Write Any Value to Clear the Output IF 1/2 Generated
 	// Interrupt
 	*/
	DISP_OUTIF1_ERR_CLEAR     = ( DISP_REG_COMMON_BASE ) + ( 4 << 2),
	DISP_OUTIF2_ERR_CLEAR     = ( DISP_REG_COMMON_BASE ) + ( 5 << 2),
 	/*
 	// Write Any Value to Clear the Compositor1/2 to OutputIF1/2
 	// FIFO Read Empty Error
 	*/
 	
	DISP_SCA_COEF_IDX         = ( DISP_REG_COMMON_BASE ) + ( 6 << 2),
	/* Write this before update Any Coeff Values
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
	//           *Note: Other Value will not take any Effect
	*/
	DISP_SCA_COEF_DATA        = ( DISP_REG_COMMON_BASE ) + ( 7 << 2),
	/* Write or Read it to Update or Retrieve a Coeff Data
	// Bit[11: 0] : Coeff Data, Reset Value 0;
	// Bit[14:12] : Reserved to be zero;
	// Bit[   15] : Signed Bit, Reset Value 0;
  	//              1: Negtive Coeff, 0: Positive Coeff
	*/
	
	/***************GFX1 Register******************/
	DISP_GFX1_CTRL            = ( DISP_REG_GFX1_BASE ) + ( 0 << 2), //0x41800040
	/*
	// Bit[1:0]  : GFX_EN,    0/3: Disable, 1: Gfx on Output1, 2: Gfx on Output2, Reset Value: 0;
	// Bit[2]    : SCALER_EN,  0: Disable, 1: Enable, Reset Value 0;
	// Bit[3]    : COLOR_KEYING_EN,  0: Disable, 1: Enable, Reset Value 0;
	// Bit[5:4]  : FETCH_TYPE, Decide the Fetch Type when Interlaced Display, no effect when Progressive display,
	//                       0/3: Auto, 1; Fetch Top Field Only, 2: Fetch Bot Field only, Reset Value: 0;
	// Bit[6]    : RGB2YUV_CONV_EN, 1: Enable the RGB to YUV Conversion, 
	//                       0: Disable the RGB to YUV Conversion, Output RGB Directly
	//                       Reset value: 1;
	// Bit[7]    : Reseverd to be Zero,
	// Bit[31:12]: Reseverd to be Zero,
 	*/
 	DISP_GFX1_FORMAT          = ( DISP_REG_GFX1_BASE ) + ( 1 << 2),  ////0x41800044
 	/*
 	// Bit[ 2: 0] : FORMAT, 
 	// Bit[ 7: 3] : Reseverd to be Zero,
	// Bit[10: 8] : GFX1_BYTE_ENDIAN, 
	//              Bit[8] : Byte_endian, 32bit Bytes Endian,  Reset Value: 0
	//              Bit[9] : word_endian, 128bit 32bit Endian, Reset Value: 1
	//              Bit[10]: 16bit Bytes Endian,  Reset Value: 1
	//              *Note  : For All Endian Bit, if 0 change bit Order, 1 unchanged
	// Bit[   11] : GFX1_NIBBLE_ENDIAN, Reset Value 0
 	*/
 	DISP_GFX1_ALPHA_CTRL       = ( DISP_REG_GFX1_BASE ) + ( 2 << 2),
 	/*
 	// Bit[ 7: 0] : DEFAULT_ALPHA, Reset Value: 0,
 	// Bit[15: 8] : Reseverd to be Zero,
 	// Bit[23:16] : ARGB1555_ALPHA0,  Reset Value: 0,
 	// Bit[31:24] : ARGB1555_ALPHA1,  Reset Value: 0,
 	*/
 	DISP_GFX1_KEY_RED         = ( DISP_REG_GFX1_BASE ) + ( 3 << 2), 
 	/*
 	// Bit[ 7: 0] : RED_MIN, Reset Value: 0,
 	// Bit[15: 8] : RED_MAX, Reset Value: 0,
 	*/
 	DISP_GFX1_KEY_BLUE        = ( DISP_REG_GFX1_BASE ) + ( 4 << 2), 
 	/*
 	// Bit[ 7: 0] : BLUE_MIN, Reset Value: 0,
 	// Bit[15: 8] : BLUE_MAX, Reset Value: 0,
 	*/
 	DISP_GFX1_KEY_GREEN       = ( DISP_REG_GFX1_BASE ) + ( 5 << 2), 
 	/*
 	// Bit[ 7: 0] : GREEN_MIN, Reset Value: 0,
 	// Bit[15: 8] : GREEN_MAX, Reset Value: 0,
 	*/
 	DISP_GFX1_BUF_START       = ( DISP_REG_GFX1_BASE ) + ( 6 << 2), 
 	/*
 	// Bit[23: 0] : Gfx FrameBuf Start Address, 
 	//              128Bit Word Address(Bytes Address >> 4), Reset Value: 0
 	*/
 	DISP_GFX1_LINE_PITCH      = ( DISP_REG_GFX1_BASE ) + ( 7 << 2), 
	/*
 	// Bit[19: 0] : Gfx FrameBuf LinePitch, 
 	//              128Bit Word Address(Bytes Address >> 4), Reset Value: 0
 	// Bit[23:20] : Reserved to be zero
 	// Bit[28:24] : blank_pixel:  Gfx Layer Blank Pixel Num in each Line Head, 
 	//              if Scaler on Gfx, it's the Destination Line Blank numbers
 	//              Reset Value: 0;
 	// Bit[31   ] : DRAM_MAPPING: Never Used in Current Version, 
 	//              Please set it to Zero, Reset Value: 0.
 	*/
 	DISP_GFX1_X_POSITON       = ( DISP_REG_GFX1_BASE ) + ( 8 << 2), 
 	/*
 	// Bit[10: 0] : X_START, Reset Value 0
 	// Bit[15:11] : Reserved to be zero
 	// Bit[26:16] : X_END, Reset Value 0
 	// Bit[31:27] : Reserved to be zero
 	//      *Note :  (1) X_END = X_START + SrcWidth - 1, 
 	//                   SrcWidth multiply with Pixel Bit Width should be a multiply of 128bits
 	//               (2) If Gfx Horizontal Scaler is not Enabled,
 	//                   X_START and X_END indict the Gfx Layer Display X Location in Screen.
 	//               (3) If Gfx Horizontal Scaler is Eanbled, these two decide the SrcWidth only
 	*/
 	DISP_GFX1_Y_POSITON       = ( DISP_REG_GFX1_BASE ) + ( 9 << 2), 
 	/*
 	// Bit[10: 0] : Y_START, Reset Value 0
 	// Bit[15:11] : Reserved to be zero
 	// Bit[26:16] : Y_END, Reset Value 0
 	// Bit[31:27] : Reserved to be zero
 	//      *Note :  (1) If Progressive Displayed, 
 	//                   Y_START = Gfx Layer Screen Y OffSet,
 	//                   Y_END   = Y_START + GfxSrcHeight;
 	//               (2) If Interlaced Displayed, 
 	//                   Y_START = Gfx Layer Screen Y OffSet / 2
 	//                   Y_END   = (Y_START + GfxSrcHeight)  / 2; 
 	*/
 	DISP_GFX1_SCL_X_POSITON   = ( DISP_REG_GFX1_BASE ) + (10 << 2), 
 	/*
 	// Bit[10: 0] : SCL_X_START, Reset Value 0
 	// Bit[15:11] : Reserved to be zero
 	// Bit[26:16] : SCL_X_END, Reset Value 0
 	// Bit[31:27] : Reserved to be zero
 	//      *Note :  (1) SCL_X_END = SCL_X_START + DispWidth - 1, 
 	//                   SrcWidth multiply with Pixel Bit Width should be a multiply of 128bits
 	//               (2) If Gfx Horizontal Scaler is not Enabled, 
 	//                   SCL_X_START and SCL_XEND is unused.
 	//               (3) If Gfx Horizontal Scaler is Eanbled, 
 	//                   SCL_X_START and SCL_XEND indict the Gfx Layer Display X Location in Screen.
 	*/
 	DISP_GFX1_CLUT_ADDR       = ( DISP_REG_GFX1_BASE ) + (11 << 2),
 	/* Clut Table Item Idx, Write it to Update or Read the correspond Item Data
 	// Bit[ 7: 0] : Clut_Idx, For Clut4, Valid Value is 0 to 15, 
 	//              for Clut8, Valid Value is 0 to 255
 	*/
 	DISP_GFX1_CLUT_DATA       = ( DISP_REG_GFX1_BASE ) + (12 << 2), 
 	/* Clut Talbe Item Data, Write it or Read it after Set the CLUT_ADDR
 	// Bit[7 : 0] : Blue
 	// Bit[15: 8] : Green
 	// Bit[23:16] : Red
 	// Bit[31:24] : Alpha
 	*/
 	/***************GFX2 Register******************/
 	/*
 	// *Note: All the Bits of GFX2 Registers are same with GFX1 Registers
 	*/
	DISP_GFX2_CTRL            = ( DISP_REG_GFX2_BASE ) + ( 0 << 2),
	DISP_GFX2_FORMAT          = ( DISP_REG_GFX2_BASE ) + ( 1 << 2),
	DISP_GFX2_ALPHA_CTRL      = ( DISP_REG_GFX2_BASE ) + ( 2 << 2),
	DISP_GFX2_KEY_RED         = ( DISP_REG_GFX2_BASE ) + ( 3 << 2), 
	DISP_GFX2_KEY_BLUE        = ( DISP_REG_GFX2_BASE ) + ( 4 << 2), 
	DISP_GFX2_KEY_GREEN       = ( DISP_REG_GFX2_BASE ) + ( 5 << 2), 
	DISP_GFX2_BUF_START       = ( DISP_REG_GFX2_BASE ) + ( 6 << 2), 
	DISP_GFX2_LINE_PITCH      = ( DISP_REG_GFX2_BASE ) + ( 7 << 2), 
	DISP_GFX2_X_POSITON       = ( DISP_REG_GFX2_BASE ) + ( 8 << 2), 
	DISP_GFX2_Y_POSITON       = ( DISP_REG_GFX2_BASE ) + ( 9 << 2), 
	DISP_GFX2_SCL_X_POSITON   = ( DISP_REG_GFX2_BASE ) + (10 << 2), 
	DISP_GFX2_CLUT_ADDR       = ( DISP_REG_GFX2_BASE ) + (11 << 2),
	DISP_GFX2_CLUT_DATA       = ( DISP_REG_GFX2_BASE ) + (12 << 2),
	
	/***************VIDEO1 Register******************/
	DISP_VIDEO1_CTRL          = ( DISP_REG_VIDEO1_BASE ) + ( 0 << 2),
	/*
	// Bit[ 1: 0]: VIDEO_EN,    0/3: Disable, 
	//                            1: Video on Output1, 
	//                            2: Video on Output2, Reset Value: 0;
	// Bit[2]    : LUMA_KEY_EN    0: Disable Luma Keying, 
	//                            1: Enable Luma Keying, Reset Value: 0;
	// Bit[3]    : COLOR_MODULATOR_EN: 
	//                            0: Disable Color Modulator, 
	//                            1: Enable Color Modulator, Reset Value 0:
	// Bit[4]    : LUMA_SCAL_VFIR_TAP_NUM_SEL: 
	//                            1: Vertical 2Tap,
	//                            0: Vertical 4Tap, Reset Value 0:
	//      *Note: the Video Source Width is indicat by the SrcCropWidth, 
	//             if SrcCropWidth > 1024, the Vertical Scaler will be set to be 0 auto,
	//             Because the Video LineBuf Width is 1024 * 4, 
	//             which can only contain 2 HD Lines and 4 SD Lines.
	// Bit[5]    : Enable Burst Divide into 2 for H264 2D Format when DDR Ctrl is 
	//             Configured  to be 16Bits mode not 32Bits mode,
	//             0, Disable (Reset State)
	//             1, Enable
	// Bit[ 7: 6]: Reseverd to be Zero,
	// Bit[31: 8]: Reseverd to be Zero,
 	*/
	DISP_VIDEO1_ALPHA_CTRL    = ( DISP_REG_VIDEO1_BASE ) + ( 1 << 2),
 	/*
 	// Bit[ 7: 0] : DEFAULT_ALPHA, Reset Value: 0,
 	// Bit[15: 8] : Reseverd to be Zero,
 	// Bit[23:16] : LUMA_KEY_ALPHA0,  Reset Value: 0,
 	// Bit[31:24] : LUMA_KEY_ALPHA1,  Reset Value: 0,
 	//      *Note : (1) If the Video Layer Luma Keying is Disabled,
 	//                  each pixel's alpha value of video layer is DEFAULT_ALPHA
 	//              (2) Luma Keying is Enabled, each pixel's alpha value is 
 	//                  decided by the keying result.
 	//                  The Luma Keying Condition is: 
 	//                  IsKeying = LUMA_KEY_MIN <= InputLuma <= LUMA_KEY_MAX
 	//                  if (IsKeying) the Pixel's Alpha is LUMA_KEY_ALPHA0,
 	//                  else the Pixel's Alpha is LUMA_KEY_ALPHA1
 	*/
 	DISP_VIDEO1_KEY_LUMA      = ( DISP_REG_VIDEO1_BASE ) + ( 2 << 2), 
 	/*
 	// Bit[ 7: 0] : LUMA_KEY_MIN, Reset Value: 0,
 	// Bit[15: 8] : LUMA_KEY_MAX, Reset Value: 0,
 	*/
 	DISP_VIDEO1_X_POSITON     = ( DISP_REG_VIDEO1_BASE ) + ( 3 << 2), 
 	/*
 	// Bit[10: 0] : X_START, Reset Value 0
 	// Bit[14:11] : X_START_CROP_PIXEL_NUM, Reset Value 0
 	// Bit[15   ] : Reserved to be zero
 	// Bit[26:16] : X_END, Reset Value 0
 	// Bit[30:27] : X_END_CROP_PIXEL_NUM, Reset Value 0
 	// Bit[31   ] : ENA_X_START_END_CROP, Reset Value 1
 	//      *Note :  (1) X_START and X_END indict the Video Layer Horizontal Location is Screen
 	//               (2) The Video Display Width = X_END - X_START
 	//               (3) X_START and X_END Can be one Pixel Aligned!
 	//      *Note : About the X_START_END CROP Pixel Number,
 	//               (1) if ENA_X_START_END_CROP is Enabled, the final display Width is 
 	//      X_END - X_START + X_START_CROP_PIXEL_NUM + X_END_CROP_PIXEL_NUM;
 	//               (2) but First TX_START_CROP_PIXEL_NUM Pixels and 
 	//      Last X_END_CROP_PIXEL_NUM pixels will be discard to displayed in the Screen.
 	//               (3) Make Sure the Total Width should not exceed 1920.
 	*/
 	DISP_VIDEO1_Y_POSITON     = ( DISP_REG_VIDEO1_BASE ) + ( 4 << 2), 
 	/*
 	// Bit[10: 0] : Y_START, Reset Value 0
 	// Bit[15:11] : Reserved to be zero
 	// Bit[26:16] : Y_END, Reset Value 0
 	// Bit[31:27] : Reserved to be zero
 	//      *Note :  (1) Y_START and Y_END indict the Video Layer Vertical Location is Screen
 	//               (2) The Video Display Height = Y_END - Y_START
 	//               (3) Y_START and X_END Must be a Multiply of 4!    
 	*/
 	DISP_VIDEO1_SRC_X_CROP    = ( DISP_REG_VIDEO1_BASE ) + ( 5 << 2),     
 	/* Define the Video Soruce Cropping Windows Horizontal Location
 	// Bit[10: 0] : CROP_X_OFF  , Reset Value 0
 	// Bit[15:11] : Reserved to be zero
 	// Bit[26:16] : CROP_X_WIDTH, Reset Value 0
 	// Bit[31:27] : Reserved to be zero
 	//      *Note :  (1) CROP_X_OFF and CROP_X_WIDTH'w lower 4Bits will be Always zero by Hardware
 	*/
  	DISP_VIDEO1_SRC_Y_CROP    = ( DISP_REG_VIDEO1_BASE ) + ( 6 << 2),
  	/* Define the Video Soruce Cropping Windows Vertical Location
 	// Bit[10: 0] : CROP_Y_OFF  , Reset Value 0
 	// Bit[15:11] : Reserved to be zero
 	// Bit[26:16] : CROP_Y_HEIGHT, Reset Value 0
 	// Bit[31:27] : Reserved to be zero
 	//      *Note :  (1) CROP_Y_OFF and CROP_Y_HEIGHT'w lower 2Bits will be Always zero by Hardware
 	*/
 	DISP_VIDEO1_CM_COEF0_012  = ( DISP_REG_VIDEO1_BASE ) + ( 7 << 2),
 	/* Video Layer Color Modulator Coeff
 	// Bit[8 : 0] : CM_COEFF00, Reset Value 0
 	// Bit[18:10] : CM_COEFF01, Reset Value 0
 	// Bit[28:20] : CM_COEFF02, Reset Value 0 
 	//       *Note: The Three Coeff is 9 Bits:
 	//              Bit[8] is Sign Bit, 0 Positive, 1 Negative
 	//              Bit[7..6] is Integer Bits
 	//              Bit[5..0] is Fraction Bits
 	*/
 	DISP_VIDEO1_CM_COEF0_3    = ( DISP_REG_VIDEO1_BASE ) + ( 8 << 2),
 	/* Video Layer Color Modulator Coeff
 	// Bit[12: 0] : CM_COEFF03, Reset Value 0
 	//       *Note: Bit[12] is Sign Bit, 0 Positive, 1 Negative
 	//              Bit[11:0] is Integer Bits
 	*/
 	DISP_VIDEO1_CM_COEF1_012  = ( DISP_REG_VIDEO1_BASE ) + ( 9 << 2),
 	/* Video Layer Color Modulator Coeff
 	// Bit[8 : 0] : CM_COEFF10, Reset Value 0
 	// Bit[18:10] : CM_COEFF11, Reset Value 0
 	// Bit[28:20] : CM_COEFF12, Reset Value 0 
 	*/
 	DISP_VIDEO1_CM_COEF1_3    = ( DISP_REG_VIDEO1_BASE ) + (10 << 2),
 	/* Video Layer Color Modulator Coeff
 	// Bit[12: 0] : CM_COEFF13, Reset Value 0
 	*/
 	DISP_VIDEO1_CM_COEF2_012  = ( DISP_REG_VIDEO1_BASE ) + (11 << 2),
 	/* Video Layer Color Modulator Coeff
 	// Bit[8 : 0] : CM_COEFF20, Reset Value 0
 	// Bit[18:10] : CM_COEFF21, Reset Value 0
 	// Bit[28:20] : CM_COEFF22, Reset Value 0 
 	*/
 	DISP_VIDEO1_CM_COEF2_3    = ( DISP_REG_VIDEO1_BASE ) + (12 << 2),
 	/* Video Layer Color Modulator Coeff
 	// Bit[12: 0] : CM_COEFF03, Reset Value 0
 	*/
 	
	/***************Video 1 STATUS(Command Info) Register******************/
 	/*  Note: 
 	//  (1) All the Status Register is Read only and do not need to be doubled
	/   (2) Most of the infomation comes from the current used video commands
	*/
 	DISP_VIDEO1_STA_IMG_SIZE   = ( DISP_REG_VIDEO1_BASE ) + (13 << 2),
 	/*
 	// Bit[10: 0] : image_width   : Reset Value 1920;
 	// Bit[15:11] : Reserved to be zero
 	// Bit[26:16] : image_height  : Reset Value 1088;
 	// Bit[31:27] : Reserved to be zero
 	*/
 	DISP_VIDEO1_STA_FRM_INFO   = ( DISP_REG_VIDEO1_BASE ) + (14 << 2),
 	/* Current Display Frame/Field Info from CMD
 	// Bit[    0] : IsHD (mem_width_sel) : Reset Value 0;
	// Bit[    1] : VIDEO_TYPE     : 0: M2VD Fmt, 1: H264 Fmt, Reset Value 0,//YTop CMD Bit[2]
	// Bit[    2] : H264_MAP       : 0: H264 1D , 1: H264 2D,  Reset Value 0,//YTop CMD Bit[3]
	//     *Note  : This Bit is Valid Onlywhen the VIDEO_TYPPE is H264;
	// Bit[    3] : cmd_fifo_full  : Reset value 0
	// Bit[    4] : is_progress_seq: Reset Value 0 //YTop CMD Bit[0]
	// Bit[    5] : is_top_field   : Reset Value 1 //YTop CMD Bit[1] Inverse
	// Bit[7 : 6] : Reserved to Be Zero
	// Bit[11: 8] : sca_ver_init_phase: Reset Value 0 //YBot CMD Bit[3:0]
	// Bit[15:12] : sca_hor_init_phase: Reset Value 0 //CTop CMD Bit[3:0]
	// Bit[20:16] : repeat_cnt        : Reset Value 0;
	// Bit[23:21] : Reserved to be zero
	// Bit[28:24] : cmd_fifo_size     : Reset Value 16;
	*/
	DISP_VIDEO1_STA_Y_TOPADDR  = ( DISP_REG_VIDEO1_BASE ) + (15 << 2),
	/*
	// Bit[3 : 0] : Reserved to be zero
	// Bit[28: 4] : o_base_addr_yt, Reset Value 0
	// Bit[31:29] : Reserved to be zero
	*/
	DISP_VIDEO1_STA_Y_BOTADDR  = ( DISP_REG_VIDEO1_BASE ) + (16 << 2),
	/*
	// Bit[3 : 0] : Reserved to be zero
	// Bit[28: 4] : o_base_addr_yb, Reset Value 0
	// Bit[31:29] : Reserved to be zero
	*/
	DISP_VIDEO1_STA_C_TOPADDR  = ( DISP_REG_VIDEO1_BASE ) + (17 << 2),
	/*
	// Bit[3 : 0] : Reserved to be zero
	// Bit[28: 4] : o_base_addr_ct, Reset Value 0
	// Bit[31:29] : Reserved to be zero
	*/
	DISP_VIDEO1_STA_C_BOTADDR  = ( DISP_REG_VIDEO1_BASE ) + (18 << 2),
	/*
	// Bit[3 : 0] : Reserved to be zero
	// Bit[28: 4] : o_base_addr_cb, Reset Value 0
	// Bit[31:29] : Reserved to be zero
	*/
	DISP_VIDEO1_STA_DISP_NUM   = ( DISP_REG_VIDEO1_BASE ) + (19 << 2),
	/* Status Register, Read only, do not need doubled
	// Bit[31: 0] : disp_num, Reset value 0
	*/
	/***************VIDEO2 Register******************/
	/* Refer the Define of Video 1 Registes*/
	DISP_VIDEO2_CTRL           = ( DISP_REG_VIDEO2_BASE ) + ( 0 << 2), 	
 	DISP_VIDEO2_ALPHA_CTRL     = ( DISP_REG_VIDEO2_BASE ) + ( 1 << 2),
 	DISP_VIDEO2_KEY_LUMA       = ( DISP_REG_VIDEO2_BASE ) + ( 2 << 2),
 	DISP_VIDEO2_X_POSITON      = ( DISP_REG_VIDEO2_BASE ) + ( 3 << 2), 
 	DISP_VIDEO2_Y_POSITON      = ( DISP_REG_VIDEO2_BASE ) + ( 4 << 2), 
 	DISP_VIDEO2_SRC_X_CROP     = ( DISP_REG_VIDEO2_BASE ) + ( 5 << 2),
 	DISP_VIDEO2_SRC_Y_CROP     = ( DISP_REG_VIDEO2_BASE ) + ( 6 << 2),
 	DISP_VIDEO2_CM_COEF0_012   = ( DISP_REG_VIDEO2_BASE ) + ( 7 << 2),
 	DISP_VIDEO2_CM_COEF0_3     = ( DISP_REG_VIDEO2_BASE ) + ( 8 << 2),
 	DISP_VIDEO2_CM_COEF1_012   = ( DISP_REG_VIDEO2_BASE ) + ( 9 << 2),
 	DISP_VIDEO2_CM_COEF1_3     = ( DISP_REG_VIDEO2_BASE ) + (10 << 2),
 	DISP_VIDEO2_CM_COEF2_012   = ( DISP_REG_VIDEO2_BASE ) + (11 << 2),
 	DISP_VIDEO2_CM_COEF2_3     = ( DISP_REG_VIDEO2_BASE ) + (12 << 2),
 	DISP_VIDEO2_STA_IMG_SIZE   = ( DISP_REG_VIDEO2_BASE ) + (13 << 2),
	DISP_VIDEO2_STA_FRM_INFO   = ( DISP_REG_VIDEO2_BASE ) + (14 << 2),
	DISP_VIDEO2_STA_Y_TOPADDR  = ( DISP_REG_VIDEO2_BASE ) + (15 << 2),
	DISP_VIDEO2_STA_Y_BOTADDR  = ( DISP_REG_VIDEO2_BASE ) + (16 << 2),
	DISP_VIDEO2_STA_C_TOPADDR  = ( DISP_REG_VIDEO2_BASE ) + (17 << 2),
	DISP_VIDEO2_STA_C_BOTADDR  = ( DISP_REG_VIDEO2_BASE ) + (18 << 2),	
	DISP_VIDEO2_STA_DISP_NUM   = ( DISP_REG_VIDEO2_BASE ) + (19 << 2),
	
 	/***************COMPOSITOR1 Register******************/
 	DISP_COMP1_CLIP           = ( DISP_REG_COMP1_BASE ) + ( 0 << 2),
 	/*
 	// Bit[    0] : YUV Clip Enable, 1: Enable, 0: Disable, Reset Value: 0
 	// Bit[15: 8] : CLIP_Y_LOW  : Reset Value:  16
 	// Bit[23:16] : CLIP_Y_RANGE: Reset Value: 219
 	// Bit[31:24] : CLIP_C_RANGE: Reset Value: 224
 	*/
 	DISP_COMP1_BACK_GROUND    = ( DISP_REG_COMP1_BASE ) + ( 1 << 2),
 	/*
 	// Bit[7 : 0] : BG_Y : Reset Value: 0x10;
 	// Bit[15: 8] : BG_U : Reset Value: 0x80;
 	// Bit[23:16] : BG_V : Reset Value: 0x80;
 	*/ 
 	DISP_COMP1_Z_ORDER        = ( DISP_REG_COMP1_BASE ) + ( 2 << 2),
 	/* Four Layer's Order, Reset Order, BackGroud, Video1, Video2, Gfx1, Gfx2
 	// Bit[1 : 0] : Video1 ZOrder, Reset Value: 0;
 	// Bit[3 : 2] : Reserved to Zero; 
 	// Bit[5 : 4] : Video2 ZOrder, Reset Value: 0;
 	// Bit[7 : 6] : Reserved to Be Zero;
 	// Bit[9 : 8] : Gfx1 ZOrder, Reset Value: 0;
 	// Bit[11:10] : Reserved to Be Zero;
 	// Bit[13:12] : Gfx2 ZOrder, Reset Value: 0;
 	// Bit[15:14] : Reserved to Be Zero;
 	*/
 	
 	/***************COMPOSITOR2 Register******************/
 	/* Refert the COMPOSITOR1 Register Define*/
 	DISP_COMP2_CLIP           = ( DISP_REG_COMP2_BASE ) + ( 0 << 2),
 	DISP_COMP2_BACK_GROUND    = ( DISP_REG_COMP2_BASE ) + ( 1 << 2),
 	DISP_COMP2_Z_ORDER        = ( DISP_REG_COMP2_BASE ) + ( 2 << 2),
 	 	
 	
 	/***************HD2SD_CAPTURE Register******************/
 	DISP_HD2SD_CTRL       	  = ( DISP_REG_HD2SD_BASE ) + ( 0 << 2), //0x41800240
 	/*
 	// Bit[    0] : HD2SD_ENABLE: 0: Disable, 1: Eanble, Reset Value: 0;
 	// Bit[    1] : COMPOSITOR_SELECT: Reset value 0;
 	//              0: Capture Compositor0, 1: Capture Compositor1;
 	// Bit[    2] : BYPASS_SCA  : 0: Do HD2SD Scaler, 1; ByPass HD2SD Scaler
 	// Bit[    3] : Reversed to be Zero;
 	// Bit[    4] : 1: Vertical Reverse Store, 0: Vertical Normal Store, 
 	//              Reset value 0;
 	// Bit[    5] : 1: Horizontal Reverse Store, 0: Horizontal Normal Store, 
 	//              Reset value 0;
 	// Bit[    6] : IS_FRAME: Set the Store Mode
 	//              0: Interlaced Store, 
 	//                 When Interlaced display, Top and Bot Field Merge to a Frame
 	//                 When Progressive display, Store Top Field into a Frame Only
 	//              1: Progressive Store,
 	//                 When Interlaced display, Top and Bot Field Store to seperated Frame
 	//                 When Progressive display, Frame store into a Frame
 	// Bit[    7] : IS_HD   : Control the LinePitch
 	//              0: LinePitch for a Frame is 1024Bytes
 	//              1: LinePitch for a Frame is 2048Bytes
 	// Bit[11: 8] : Store Dram Buffer FIFO Depth Minus 1, Reset Value 0,
 	// Bit[15:12] : Reversed to be Zero;
 	// Bit[19:16] : Vertical Initial Phase for Scaler, Max 15, Reset Value 0;
 	// Bit[23:20] : Horizontal Initial Phase for Scaler, Max 15, Reset Value 0;
 	// Bit[31:24] : Reversed to be Zero;
 	*/
 	DISP_HD2SD_DES_SIZE       = ( DISP_REG_HD2SD_BASE ) + ( 1 << 2), ////0x41800244
 	/*
 	// Bit[10: 0] : Capture Destination Width , Should be 8Pixel Aligned,
 	//              Reset Value 0;
 	// Bit[26:16] : Capture Destination Height, Should be 2 Lines Aligned,
 	//       *Note: IF Interlaced Display, the Height should be a Field Height
 	*/
 	DISP_HD2SD_ADDR_Y         = ( DISP_REG_HD2SD_BASE ) + ( 2 << 2),
 	/* The Fisrt Caputure Buffer Luma Start Address 
 	// Bit[24: 0] : 64Bit Word Address, Reset Value 0, Require 1024Bytes Aligned?
 	*/
 	DISP_HD2SD_ADDR_C         = ( DISP_REG_HD2SD_BASE ) + ( 3 << 2),
 	/* The Fisrt Caputure Buffer Chroma Start Address
 	// Bit[24: 0] : 64Bit Word Address, Reset Value 0
 	*/
 	DISP_HD2SD_BUF_PITCH      = ( DISP_REG_HD2SD_BASE ) + ( 4 << 2),
 	/* Capture Frame Buffer Pitch
 	// Bit[24: 0] : 64Bit Word Address, Reset Value 0
 	*/
 	DISP_HD2SD_STATUS         = ( DISP_REG_HD2SD_BASE ) + ( 5 << 2),
 	/* HD2SD Runtime Status, Read only, do not need to be Doubled
 	// Bit[3 : 0]: hd2sd_current_buf, Reset 0, Update after capture one Frame
 	// Bit[31: 4]: Reserved to be Zero
 	*/
 	/***************OUTIF1 Register******************/
 	DISP_OUTIF1_CTRL          = ( DISP_REG_OUTIF1_BASE ) + ( 0 << 2),
 	/*
 	// Bit[    0] : DISP_EN: Reset Value 0,
 	//              0: Disable Current Output, 1: Enable Current Output;
 	// Bit[3 : 1] : CLK_OUT_ENA: 
 	//              Bit[1]: 0: CCIR_656 Signal Out Disable, 1: Enable, Reset Value 1;
 	//              Bit[2]: 0: CVE5 Signal Out Disable, 1: Enable, Reset Value 1;
 	//              Bit[3]: 0: VGA Signal Out Disable, 1: Eanble, Reset Value 0;
 	// Bit[    4] : IS_HD: 0: Output SD Signal, 1: Output HD Signal, Reset Value 0;
 	// Bit[    5] : IS_INTERLACED: Reset Value 0;
 	//              0: Output Progressive Signal, 1: Output Interlaced Signal
 	// Bit[    6] : NEED_REPEAT: Reset Value 0;
 	// Bit[    7] : NEED_MUX   : Reset Value 0;
 	// Bit[    8] : CCIR_F0_IS_HALFLINE: Reset Value 0;
 	// Bit[    9] : CCIR_F1_IS_HALFLINE: Reset Value 0;
 	// Bit[   10] :  VGA_F0_IS_HALFLINE: Reset Value 0;
 	// Bit[   11] :  VGA_F1_IS_HALFLINE: Reset Value 0;
 	// Bit[   12] : H_SYNC_POLARITY: Reset Value 0;
 	// Bit[   13] : V_SYNC_POLARITY: Reset Value 0;
 	// Bit[   14] : DE_POLARITY    : Reset Value 0; 
 	// Bit[   15] : IS_PAL: Reset Value 0;
 	// Bit[   16] : TopField/Frame Interrupt Enable, 1: Enable, 0 Disable, Reset 0;
 	// Bit[   17] : BotField Interrupt Enable, 1: Enable, 0 Disable, Reset 0;
 	//      *Note : (1) Top Field /Frame Interrupt happened when ??
 	//              (2) Bot Field /Frame Interrupt happened when ??
 	// Bit[   18] : YUV444 to YUV422 ChromaConvert Type, 
 	//              0: Average(U0 + U1 /2 ),
 	//              1: Drop (U0 Only, Drop U1),
 	//              Reset Value 0;
 	// Bit[   19] : ddr_RGB2_fmt: Double Data Rate Format,Reset Value 0
 	// Bit[   20] : is_rgb_fmt  : Double Data Rate Data is RGB(1) or YUV(0),Reset Value 0
 	// Bit[   21] : sync_sep    : 1: CCIR656 Sync Single Separated, 0: Embedded, Reset Value 0
 	// Bit[31:19] : Reserved to be zero
 	*/
 	DISP_OUTIF1_X_SIZE        = ( DISP_REG_OUTIF1_BASE ) + ( 1 << 2),
 	/*
 	// Bit[11: 0] : X_TOTAL     : Reset Value 0;
 	// Bit[15:12] : Reserved to be Zero;
 	// Bit[27:16] : X_ACT_START : Reset Value 0;
 	// Bit[31:28] : Reserved to be Zero;
 	*/
 	DISP_OUTIF1_Y_TOTAL       = ( DISP_REG_OUTIF1_BASE ) + ( 2 << 2),
 	/*
 	// Bit[10: 0] : Y_TOTAL
 	// Bit[31:11] : Reserved to be Zero
 	*/
 	DISP_OUTIF1_ACTIVE_TOP    = ( DISP_REG_OUTIF1_BASE ) + ( 3 << 2),
 	/*
 	// Bit[10: 0] : ACT_START_TOP : Reset Value 0;
 	// Bit[15: 7] : Reserved to be zero
 	// Bit[26:16] : ACT_END_TOP   : Reset Value 0;
 	// Bit[31:27] : Reserved to be zero
 	*/
 	DISP_OUTIF1_ACTIVE_BOT    = ( DISP_REG_OUTIF1_BASE ) + ( 4 << 2), 	 	
 	/*
 	// Bit[10: 0] : ACT_START_BOT : Reset Value 0;
 	// Bit[15: 7] : Reserved to be zero
 	// Bit[26:16] : ACT_END_BOT   : Reset Value 0;
 	// Bit[31:27] : Reserved to be zero
 	*/
 	DISP_OUTIF1_BLANK_LEVEL   = ( DISP_REG_OUTIF1_BASE ) + ( 5 << 2),
 	/*
 	// Bit[ 7: 0] : Y/R_BLANK_LEV   : Reset Value 0;
 	// Bit[15: 8] : U/G_BLANK_LEV   : Reset Value 0;
 	// Bit[23:16] : V/B_BLANK_LEV   : Reset Value 0;
 	*/
 	DISP_OUTIF1_CCIR_F_START  = ( DISP_REG_OUTIF1_BASE ) + ( 6 << 2),
 	/*
 	// Bit[10: 0] : F0_START      : Reset Value 0;
 	// Bit[15: 7] : Reserved to be zero
 	// Bit[26:16] : F1_START      : Reset Value 0;
 	// Bit[31:27] : Reserved to be zero
 	*/
 	DISP_OUTIF1_HSYNC         = ( DISP_REG_OUTIF1_BASE ) + ( 7 << 2),
 	/*
 	// Bit[11: 0] : X_H_SYNC_START: Reset Value 0;
 	// Bit[15:12] : Reserved to be Zero;
 	// Bit[27:16] : X_H_SYNC_END  : Reset Value 0;
 	// Bit[31:28] : Reserved to be Zero;
 	*/
 	DISP_OUTIF1_VSYNC_TOP     = ( DISP_REG_OUTIF1_BASE ) + ( 8 << 2),
 	/*
 	// Bit[10: 0] : Y_V_SYNC_START_TOP: Reset Value 0;
 	// Bit[15: 7] : Reserved to be zero
 	// Bit[26:16] : Y_V_SYNC_END_TOP  : Reset Value 0;
 	// Bit[31:27] : Reserved to be zero
 	*/
 	DISP_OUTIF1_VSYNC_BOT     = ( DISP_REG_OUTIF1_BASE ) + ( 9 << 2),
 	/*
 	// Bit[10: 0] : Y_V_SYNC_START_BOT: Reset Value 6;
 	// Bit[15: 7] : Reserved to be zero
 	// Bit[26:16] : Y_V_SYNC_END_BOT  : Reset Value 6;
 	// Bit[31:27] : Reserved to be zero
 	*/
 	DISP_OUTIF1_STA_DISP_SIZE = ( DISP_REG_OUTIF1_BASE ) + (10 << 2),
 	/* Runtime Status, read only, donot need to be doubled
 	// Bit[10: 0] : DISP_WIDTH : Reset Value 0;
 	// Bit[15: 7] : Reserved to be zero
 	// Bit[26:16] : DISP_HEIGHT: Reset Value 0;
 	// Bit[31:27] : Reserved to be zero
 	*/
 	DISP_OUTIF1_STA_LINE      = ( DISP_REG_OUTIF1_BASE ) + (11 << 2),
 	/* Runtime Status, read only, donot need to be doubled
 	// Bit[    0] : sys_bottom_flag: Reset Value 0
 	//              0: Current Display is Top Field or Frame
 	//              1: Current Display is Bot Field
 	// Bit[    1] : sys_frame_sync : Reset Value 0
 	// Bit[    2] : sys_vsync      : Reset Value 0
 	// Bit[15: 3] : Reserved to be zero
 	// Bit[27:16] : sys_line_count : Reset Value 0
 	//              Current Display v_cnt
 	// Bit[31:28] : Reserved to be zero
 	*/ 	
 	/***************OUTIF2 Register******************/
 	DISP_OUTIF2_CTRL          = ( DISP_REG_OUTIF2_BASE ) + ( 0 << 2),
 	DISP_OUTIF2_X_SIZE        = ( DISP_REG_OUTIF2_BASE ) + ( 1 << 2),
 	DISP_OUTIF2_Y_TOTAL       = ( DISP_REG_OUTIF2_BASE ) + ( 2 << 2),
 	DISP_OUTIF2_ACTIVE_TOP    = ( DISP_REG_OUTIF2_BASE ) + ( 3 << 2),
 	DISP_OUTIF2_ACTIVE_BOT    = ( DISP_REG_OUTIF2_BASE ) + ( 4 << 2),
 	DISP_OUTIF2_BLANK_LEVEL   = ( DISP_REG_OUTIF2_BASE ) + ( 5 << 2),
 	DISP_OUTIF2_CCIR_F_START  = ( DISP_REG_OUTIF2_BASE ) + ( 6 << 2),
 	DISP_OUTIF2_HSYNC         = ( DISP_REG_OUTIF2_BASE ) + ( 7 << 2),
 	DISP_OUTIF2_VSYNC_TOP     = ( DISP_REG_OUTIF2_BASE ) + ( 8 << 2),
 	DISP_OUTIF2_VSYNC_BOT     = ( DISP_REG_OUTIF2_BASE ) + ( 9 << 2),
 	DISP_OUTIF2_STA_DISP_SIZE = ( DISP_REG_OUTIF2_BASE ) + (10 << 2),
 	DISP_OUTIF2_STA_LINE      = ( DISP_REG_OUTIF2_BASE ) + (11 << 2),

	DISP_REG_SIZE = 0x1000,
};

#endif
