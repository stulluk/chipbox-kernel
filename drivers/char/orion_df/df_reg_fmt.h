#ifndef __DF_REG_FMT_H__
#define __DF_REG_FMT_H__

typedef struct _DF_CTRL_REG_PARA_
{
	union{
		struct {
			unsigned int iUpdateReg:1;
			unsigned int reserved:31;
		}bits;

		unsigned int val;
	}df_update_reg;
	//int iUpdateReg;

	union{
		struct {
			unsigned int oOutIF1Err:1;
			unsigned int oOutIF1ErrType:7;
			unsigned int oOutIF2Err:1;
			unsigned int oOutIF2ErrType:7;
			unsigned int oOutIF1FrmSyncInterrupt:1;
			unsigned int oOutIF2FrmSyncInterrupt:1;
			unsigned int reserved:14;
		}bits;

		unsigned int val;
	}df_status_reg;
	//int oOutIF1Err;
	//int oOutIF1ErrType;
	//int oOutIF2Err;
	//int oOutIF2ErrType;
	//int oOutIF1FrmSyncInterrupt;
	//int oOutIF2FrmSyncInterrupt;

	int iOutIF1IntClr;
	int iOutIF2IntClr;
	int iOutIF1ErrClr;
	int iOutIF2ErrClr;

	union{
		struct {
			unsigned int iScalerCoefTapIdx:2;
			unsigned int reserved_1:6;
			unsigned int iScalerCoefPhaseIdx:4;
			unsigned int reserved_2:4;
			unsigned int iScalerCoefType:3;
			unsigned int reserved_3:13;
		}bits;

		unsigned int val;
	}df_sca_coef_idx_reg;
	//int iScalerCoefTapIdx;
	//int iScalerCoefPhaseIdx;
	//int iScalerCoefType;

	int iScalerCoefData;
	
}df_ctrl_reg;

typedef struct _DF_GFX_REG_PARA_
{
	union{
		struct{
			unsigned int iGfxEnable:2;
			unsigned int iGfxScalerEnable:1;
			unsigned int iColorKeyEnable:1;
			unsigned int iFetchType:2;
			unsigned int iRGB2YUVConvertEna:1;
			unsigned int reserved:25;
		}bits;

		unsigned int val;
	}df_gfx_control_reg;
	//int iGfxEnable;
	//int iGfxScalerEnable;
	//int iColorKeyEnable;
	//int iFetchType;
	//int iRGB2YUVConvertEna;

	union {
		struct{
			unsigned int iColorFormat:3;
			unsigned int reserved_1:5;
			unsigned int iByteEndian:1;
			unsigned int  i128BitEndian:1;
			unsigned int i16BitEndian:1;
			unsigned int iNibbleEndian:1;
			unsigned int reserved_2:20;
		}bits;
		
		unsigned int val;
	}df_gfx_format_reg;
	//int iColorFormat;
	//int iByteEndian;
	//int i128BitEndian;
	//int i16BitEndian;
	//int iNibbleEndian;

	union {
		struct{
			unsigned int iDefaultAlpha:8;
			unsigned int reserved:8;
			unsigned int iArgb1555Alpha0:8;
			unsigned int iArgb1555Alpha1:8;
		}bits;
		
		unsigned int val;
	}df_gfx_alpha_control_reg;
	//U8  iDefaultAlpha;
	//U8  iArgb1555Alpha0;
	//U8  iArgb1555Alpha1;

	union {
		struct{
			unsigned int iKeyRedMin:8;
			unsigned int iKeyRedMax:8;
			unsigned int reserved:16;
		}bits;

		unsigned int val;
	}df_gfx_key_red_reg;
	//U8  iKeyRedMin;
	//U8  iKeyRedMax;

	union {
		struct{
			unsigned int iKeyBlueMin:8;
			unsigned int iKeyBlueMax:8;
			unsigned int reserved:16;
		}bits;

		unsigned int val;
	}df_gfx_key_blue_reg;
	//U8  iKeyBlueMin;
	//U8  iKeyBlueMax;

	union {
		struct{
			unsigned int iKeyGreenMin:8;
			unsigned int iKeyGreenMax:8;
			unsigned int reserved:16;
		}bits;

		unsigned int val;
	}df_gfx_key_green_reg;
	//U8  iKeyGreenMin;
	//U8  iKeyGreenMax;

	union {
		struct{
			unsigned int iStartAddr:24;
			unsigned int reserved:8;
		}bits;

		unsigned int val;
	}df_gfx_buf_start_addr_reg;
	//unsigned int iStartAddr; //WordAddr

	union {
		struct{
			unsigned int iLinePitch:20;
			unsigned int reserved_1:4;
			unsigned int iBlankPixel:5;
			unsigned int reserved_2:3;
		}bits;

		unsigned int val;
	}df_gfx_line_pitch_reg;
	//unsigned int iLinePitch; //WordAddr
	//unsigned int iBlankPixel;
	//I32 iDramMapping; //Neaver used

	union {
		struct{
			unsigned int iXStart:11;
			unsigned int reserved_1:5;
			unsigned int iXEnd:11;
			unsigned int reserved_2:5;
		}bits;
		
		unsigned int val;
	}df_gfx_x_position_reg;
	//unsigned int iXStart;
	//unsigned int iXEnd;

	union {
		struct{
			unsigned int iYStart:11;
			unsigned int reserved_1:5;
			unsigned int iYEnd:11;
			unsigned int reserved_2:5;
		}bits;
		
		unsigned int val;
	}df_gfx_y_position_reg;
	//unsigned int iYStart;
	//unsigned int iYEnd;

	union {
		struct{
			unsigned int iScaleXStart:11;
			unsigned int reserved_1:5;
			unsigned int iScaleXEnd:11;
			unsigned int reserved_2:5;
		}bits;
		
		unsigned int val;
	}df_gfx_scl_x_position_reg;
	//unsigned int iScaleXStart;
	//unsigned int iScaleXEnd;

	union {
		struct{
			unsigned int iClutAddr:8;
			unsigned int reserved:24;
		}bits;

		unsigned int val;
	}df_gfx_clut_addr_reg;
	//unsigned int iClutAddr;

	union {
		struct{
			unsigned int iblue:8;
			unsigned int ired:8;
			unsigned int igreen:8;
			unsigned int ialpha:8;
		}bits;

		unsigned int val;
	}df_gfx_clut_data_reg;
	//unsigned int iClutData;
}df_gfx_reg;

typedef struct _DF_VIDEO_REG_PARA_
{
	union {
		struct{
			unsigned int iVideoEna:2;
			unsigned int iLumaKeyEna:1;
			unsigned int iColorModulatorEna:1;
			unsigned int iLumaScleVFIRTapNumSel:1;
			unsigned int reserved:25;
		}bits;

		unsigned int val;
	}df_video_control_reg;
	//int iVideoEna;
	//int iLumaKeyEna;
	//int iColorModulatorEna;
	//int iLumaScleVFIRTapNumSel;

	union {
		struct{
			unsigned int iDefaultAlpha:8;
			unsigned int reserved:8;
			unsigned int iLumaKeyAlpha0:8;
			unsigned int iLumaKeyAlpha1:8;
		}bits;

		unsigned int val;
	}df_video_alpha_control_reg;
	//U8  iDefaultAlpha;
	//U8  iLumaKeyAlpha0;
	//U8  iLumaKeyAlpha1;

	union {
		struct{
			unsigned int iLumaKeyMin:8;
			unsigned int iLumaKeyMax:8;
			unsigned int reserved:16;
		}bits;

		unsigned int val;
	}df_video_luma_key_reg;
	//U8  iLumaKeyMin;
	//U8  iLumaKeyMax;

	union {
		struct{
			unsigned int iDispXStart:11;
			unsigned int iDispXStartCropPixelNum:4;
			unsigned int reserved:1;
			unsigned int iDispXEnd:11;
			unsigned int iDispXEndCropPixelNum:4;
			unsigned int iDispXCropEnable:1;
		}bits;

		unsigned int val;
	}df_video_x_position_reg;
	//unsigned int iDispXStart;
	//unsigned int iDispXEnd;
	//int iDispXCropEna;
	//unsigned int iDispXStartCrop;
	//unsigned int iDispXEndCrop;

	union {
		struct{
			unsigned int iDispYStart:11;
			unsigned int reserved_1:5;
			unsigned int iDispYEnd:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_video_y_position_reg;
	//unsigned int iDispYStart;
	//unsigned int iDispYEnd;

	union {
		struct{
			unsigned int iSrcCropXOff:11;
			unsigned int reserved_1:5;
			unsigned int iSrcCropXWidth:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_video_src_x_crop_reg;
	//unsigned int iSrcCropXOff;
	//unsigned int iSrcCropXWidth;

	union {
		struct{
			unsigned int iSrcCropYOff:11;
			unsigned int reserved_1:5;
			unsigned int iSrcCropYHeight:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_video_src_y_crop_reg;
	//unsigned int iSrcCropYOff;
	//unsigned int iSrcCropYHeight;

	union {
		struct{
			unsigned int coeff0:9;
			unsigned int reserved_1:1;
			unsigned int coeff1:9;
			unsigned int reserved_2:1;
			unsigned int coeff2:9;
			unsigned int reserved_3:3;
		}bits;

		unsigned int val;
	}df_video_cm_coeff0_012;

	union {
		struct{
			unsigned int coeff3:13;
			unsigned int reserved:19;
		}bits;

		unsigned int val;
	}df_video_cm_coeff0_3;

	union {
		struct{
			unsigned int coeff0:9;
			unsigned int reserved_1:1;
			unsigned int coeff1:9;
			unsigned int reserved_2:1;
			unsigned int coeff2:9;
			unsigned int reserved_3:3;
		}bits;

		unsigned int val;
	}df_video_cm_coeff1_012;

	union {
		struct{
			unsigned int coeff3:13;
			unsigned int reserved:19;
		}bits;

		unsigned int val;
	}df_video_cm_coeff1_3;

	union {
		struct{
			unsigned int coeff0:9;
			unsigned int reserved_1:1;
			unsigned int coeff1:9;
			unsigned int reserved_2:1;
			unsigned int coeff2:9;
			unsigned int reserved_3:3;
		}bits;

		unsigned int val;
	}df_video_cm_coeff2_012;

	union {
		struct{
			unsigned int coeff3:13;
			unsigned int reserved:19;
		}bits;

		unsigned int val;
	}df_video_cm_coeff2_3;
	//unsigned int iCMCoeff[3][4];

	union {
		struct{
			unsigned int oSrcFrameWidth:11;
			unsigned int reserved_1:5;
			unsigned int oSrcFrameHeight:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_video_status_frame_size;
	//unsigned int oSrcFrameWidth;
	//unsigned int oSrcFrameHeight;

	union {
		struct{
			unsigned int oSrcIsHD:1;
			unsigned int oSrcVideoType:1;
			unsigned int oSrcH264Map:1;
			unsigned int oIsCMDFifoFull:1;
			unsigned int oSrcIsProgressSeq:1;
			unsigned int oSrcIsFrmFld:1;
			unsigned int reserved_1:2;
			unsigned int oScalerVInitPhase:4;
			unsigned int oScalerHInitPhase:4;
			unsigned int oRepeatCnt:5;
			unsigned int reserved_2:3;
			unsigned int oCMDFIFOSize:5;
			unsigned int reserved_3:3;
		}bits;

		unsigned int val;
	}df_video_status_frame_info;
	//int oSrcIsHD;
	//int oSrcVideoType;
	//int oSrcH264Map;
	//int oIsCMDFifoFull;
	//int oSrcIsFrmFld;
	//int oSrcTopBotId;
	//int oScalerVInitPhase;
	//int oScalerHInitPhase;
	//int oRepeatCnt;
	//int oCMDFIFOSize;

	union {
		struct{
			unsigned int reserved_1:4;
			unsigned int oSrcYTopAddr:25;
			unsigned int reserved_2:3;
		}bits;
		
		unsigned int val;
	}df_video_status_y_topaddr;
	//unsigned int oSrcYTopAddr; //Word Address;

	union {
		struct{
			unsigned int reserved_1:4;
			unsigned int oSrcYTopAddr:25;
			unsigned int reserved_2:3;
		}bits;
		
		unsigned int val;
	}df_video_status_y_botaddr;
	//unsigned int oSrcYBotAddr;

	union {
		struct{
			unsigned int reserved_1:4;
			unsigned int oSrcCTopAddr:25;
			unsigned int reserved_2:3;
		}bits;
		
		unsigned int val;
	}df_video_status_c_topaddr;
	//unsigned int oSrcCTopAddr;

	union {
		struct{
			unsigned int reserved_1:4;
			unsigned int oSrcCBotAddr:25;
			unsigned int reserved_2:3;
		}bits;
		
		unsigned int val;
	}df_video_status_c_botaddr;
	//unsigned int oSrcCBotAddr;

	unsigned int oVideoDispNum;	
}df_video_reg;

typedef struct _DF_COMPOSITOR_REG_PARA_
{
	union {
		struct{
			unsigned int iClipEna:8;
			unsigned int iClipYLow:8;
			unsigned int iClipYRange:8;
			unsigned int iClipCRange:8;
		}bits;

		unsigned int val;
	}df_comp_clip;
	//int iClipEna;
	//U8  iClipYLow;
	//U8  iClipYRange;
	//U8  iClipCRange;

	union {
		struct{
			unsigned int iBGY:8;
			unsigned int iBGU:8;
			unsigned int iBGV:8;
			unsigned int reserved:8;
		}bits;

		unsigned int val;
	}df_comp_back_ground;
	//U8  iBGY;
	//U8  iBGU;
	//U8  iBGV;

	union{
		struct{
			unsigned int iVideo1ZOrder:2;
			unsigned int reserved_1:2;
			unsigned int iVideo2ZOrder:2;
			unsigned int reserved_2:2;
			unsigned int iGfx1ZOrder:2;
			unsigned int reserved_3:2;
			unsigned int iGfx2ZOrder:2;
			unsigned int reserved_4:18;
		}bits;

		unsigned int val;
	}df_comp_z_order;
	//int iVideo1ZOrder;
	//int iVideo2ZOrder;
	//int iGfx1ZOrder;
	//int iGfx2ZOrder;
}df_compositor_reg;

typedef struct _DF_HD2SD_REG_PARA_
{
	union{
		struct{
			unsigned int iHD2SDEna:1;
			unsigned int iCompositorSel:1;
			unsigned int iByPassScaler:1;
			unsigned int reserved_1:1;
			unsigned int iVerticalReverseStore:1;
			unsigned int iHorizontalReverseStore:1;
			unsigned int iIsFrame:1;
			unsigned int iIsHD:1;
			unsigned int iDramFIFODepthMinus1:4;
			unsigned int reserved_2:4;
			unsigned int iVInitPhase:4;
			unsigned int iHInitPhase:4;
			unsigned int reserved_3:8;
		}bits;

		unsigned int val;
	}df_hd2sd_control;
	//int iHD2SDEna;
	//int iCompositorSel;
	//int iByPassScaler;
	//int iVerticalReverseStore;
	//int iHorizontalReverseStore;
	//int iIsFrame;
	//int iIsHD;
	//int iDramFIFODepthMinus1;
	//int iVInitPhase;
	//int iHInitPhase;

	union {
		struct{
			unsigned int iDesWidth:11;
			unsigned int reserved_1:5;
			unsigned int iDesHeight:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_hd2sd_des_size;
	//unsigned int iDesWidth;
	//unsigned int iDesHeight;

	unsigned int iDesYAddr;

	unsigned int  iDesCAddr;

	unsigned int  iDesBufPith;

	int oCurBufIdx;
}df_hd2sd_reg;

typedef struct _DF_OUTIF_REG_PARA
{
	union {
		struct{
			unsigned int iDispEna:1;
			unsigned int iClkOutSel:3;
			unsigned int iIsHD:1;
			unsigned int iIsInterlaced:1;
			unsigned int iNeedRepeat:1;
			unsigned int iNeedMux:1;
			unsigned int iCVE5F0IsHalfLine:1;
			unsigned int iCVE5F1IsHalfLine:1;
			unsigned int iVgaF0IsHalfLine:1;
			unsigned int iVgaF1IsHalfLine:1;
			unsigned int iHSyncPolarity:1;
			unsigned int iVSyncPolarity:1;
			unsigned int iDEPolarity:1;
			unsigned int iIsPal:1;
			unsigned int iTopOrFrameIntEna:1;
			unsigned int iBotIntEna:1;
			unsigned int iChoromaDrop:1;
			unsigned int ddr_RGB2_fmt:1;
			unsigned int is_rgb_fmt:1;
			unsigned int sync_sep:1;
			unsigned int reserved:10;
		}bits;

		unsigned int val;
	}df_outif_control;
	//int iDispEna;
	//int iClkCCIR656Ena;
	//int iClkCVE5Ean;
	//int iClkVGAEna;
	//int iIsHD;
	//int iIsInterlaced;
	//int iNeedRepeat;
	//int iNeedMux;
	//int iCVE5F0IsHalfLine;
	//int iCVE5F1IsHalfLine;
	//int iVgaF0IsHalfLine;
	//int iVgaF1IsHalfLIne;
	//int iHSyncPolarity;
	//int iVSyncPolarity;
	//int iDEPolarity;
	//int iIsPal;
	//int iTopOrFrameIntEna;
	//int iBotIntEna;
	//int iChoromaDrop;
	//
	//int ddr_RGB2_fmt;
	//int is_rgb_fmt;
	//int sync_sep;

	union {
		struct{
			unsigned int iXTotal:12;
			unsigned int reserved_1:4;
			unsigned int iXActiveStart:12;
			unsigned int reserved_2:4;
		}bits;

		unsigned int val;
	}df_outif_x_size;
	//unsigned int iXTotal;
	//unsigned int iXActiveStart;

	union {
		struct{
			unsigned int iYTotal:11;
			unsigned int reserved:21;
		}bits;

		unsigned int val;
	}df_outif_y_size;
	//unsigned int iYTotal;

	union {
		struct{
			unsigned int iYTopActiveStart:11;
			unsigned int reserved_1:5;
			unsigned int iYTopActiveEnd:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_outif_active_top;
	//unsigned int iYTopActiveStart;
	//unsigned int iYTopActiveEnd;

	union {
		struct{
			unsigned int iYBotActiveStart:11;
			unsigned int reserved_1:5;
			unsigned int iYBotActiveEnd:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_outif_active_bot;
	//unsigned int iYBotActiveStart;
	//unsigned int iYBotActiveEnd;

	union {
		struct{
			unsigned int iYRBlankLevel:8;
			unsigned int iUGBlankLevel:8;
			unsigned int iVBBlankLevel:8;
			unsigned int reserved:8;
		}bits;

		unsigned int val;
	}df_outif_blank_level;
	//U8  iYRBlankLevel;
	//U8  iUGBlankLevel;
	//U8  iVBBlankLevel;

	union {
		struct{
			unsigned int iCCIRF0Start:11;
			unsigned int reserved_1:5;
			unsigned int iCCIRF1Start:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_outif_ccir_f_start;
	//unsigned int iCCIRF0Start;
	//unsigned int iCCIRF1Start;

	union {
		struct{
			unsigned int iHSyncStart:12;
			unsigned int reserved_1:4;
			unsigned int iHSyncEnd:12;
			unsigned int reserved_2:4;
		}bits;

		unsigned int val;
	}df_outif_h_sync;
	//unsigned int iHSyncStart;
	//unsigned int iHSyncEnd;

	union {
		struct{
			unsigned int iVSyncTopStart:11;
			unsigned int reserved_1:5;
			unsigned int iVSyncTopEnd:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_outif_v_sync_top;
	//unsigned int iVSyncTopStart;
	//unsigned int iVSyncTopEnd;

	union {
		struct{
			unsigned int iVSyncBotStart:11;
			unsigned int reserved_1:5;
			unsigned int iVSyncBotEnd:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_outif_v_sync_bot;
	//unsigned int iVSyncBotStart;
	//unsigned int iVSyncBotEnd;

	union {
		struct{
			unsigned int oDispWidth:11;
			unsigned int reserved_1:5;
			unsigned int oDispHeight:11;
			unsigned int reserved_2:5;
		}bits;

		unsigned int val;
	}df_outif_status_disp_size;
	//unsigned int oDispWidth;
	//unsigned int oDispHeight;

	union {
		struct{
			unsigned int oBotFlag:1;
			unsigned int oFrameSync:1;
			unsigned int oVSync:1;
			unsigned int reserved_1:13;
			unsigned int oLineCnt:12;
			unsigned int reserved_2:4;
		}bits;

		unsigned int val;
	}df_outif_status_line;
	//int oBotFlag;  //Not Used in Cmodel
	//int oFrameSync;//Not Used in Cmodel
	//int oVSync;    //Not Used in Cmodel
	//int oLineCnt;  //Not Used in Cmodel
}df_outif_reg;

typedef struct _DF_REG_PARA_
{
	df_ctrl_reg  Ctrl;
	df_gfx_reg   Gfx[2];
	df_video_reg Video[2];
	df_compositor_reg Comp[2];
	df_hd2sd_reg HD2SD;
	df_outif_reg OutIF[2];
}df_reg_para;

#endif
