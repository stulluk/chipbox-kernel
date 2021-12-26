#ifndef __CSAPI_DEMUX_H__
#define __CSAPI_DEMUX_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
        OUTPUT_MODE_CVBS_SVIDEO = 0, /* TVE0 not support */
        OUTPUT_MODE_YPBPR,
        OUTPUT_MODE_RGB/* TVE1 not support */
} TVOUT_OUTPUT_MODE;

typedef enum
{
	NOT_STANDARD_VIDEO_MODE = -1,		
	DISP_YUV_NTSC        =  0,
	DISP_YUV_720P_60FPS  =  1,
	DISP_YUV_720P_50FPS  =  2,
	DISP_YUV_1080I_30FPS =  3,
	DISP_YUV_1080I_25FPS =  4,
	DISP_YUV_PAL         =  5,
	DISP_YUV_480P        =  6,
	DISP_YUV_576P        =  7,
	DISP_YUV_1080P_30FPS =  8,
	DISP_YUV_1080P_25FPS =  9,
	DISP_YUV_1080P_60FPS = 10,
	DISP_YUV_1080P_50FPS = 11,
	
	DISP_RGB_640X480_60FPS = 12,//0x8616
	DISP_RGB_800X600_60FPS = 13,//0x882f
	DISP_RGB_800X600_72FPS = 14,//0x883b
	DISP_RGB_1024X768_60FPS = 15,//0x863a
	DISP_RGB_1280X1024_50FPS = 16,//0x886a
	DISP_RGB_1600X1000_60FPS = 17,
       DISP_RGB_1280X1024_60FPS = 18,//0x8660
       DISP_YUV_1080P_24FPS = 19,
       DISP_RGB_1024X768_70FPS = 20,
       DISP_RGB_1280x720_60FPS =21,
       DISP_RGB_848X480_60FPS = 22,
       DISP_RGB_800X480_60FPS = 23, 
 	DISP_FMT_MAX,
}DF_VIDEO_FORMAT;

typedef enum 
{
	TVOUT_MODE_576I = 0,
	TVOUT_MODE_480I,
	TVOUT_MODE_576P,
	TVOUT_MODE_480P,
	TVOUT_MODE_720P50,
	TVOUT_MODE_720P60,
	TVOUT_MODE_1080I25,
	TVOUT_MODE_1080I30,

	TVOUT_MODE_SECAM,
	TVOUT_MODE_PAL_M,
	TVOUT_MODE_PAL_N,
	TVOUT_MODE_PAL_CN,
	TVOUT_MODE_1080P24,
	TVOUT_MODE_1080P25,
	TVOUT_MODE_1080P30,

	TVOUT_RGB_640X480_60FPS,
	TVOUT_RGB_800X600_60FPS,
	TVOUT_RGB_800X600_72FPS,
	TVOUT_RGB_1024X768_60FPS,
	TVOUT_RGB_1280X1024_50FPS,
	TVOUT_RGB_1600X1000_60FPS,
       TVOUT_RGB_1280X1024_60FPS,
       TVOUT_RGB_1280X720_60FPS,
       TVOUT_RGB_848X480_60FPS,
       TVOUT_RGB_800X480_60FPS,
} TVOUT_MODE;

typedef enum
{
	TVE_NOT_STANDARD_VIDEO_MODE = -1,
	TVE_IS_PAL 	        = 3,
	TVE_IS_NTSC		    =  0,
	TVE_IS_1080I		    =  2, 
	TVE_IS_1080I_25FPS     =  4, 
	TVE_IS_720P	 	    =  1, 
	TVE_IS_720P_50FPS	    =  5, 
	TVE_IS_USR_DEF         =  7,
	TVE_IS_480P            =  ((1<< 4) | 7), //
	TVE_IS_576P            =  ((2<< 4) | 7), 
	TVE_IS_576I            =  ((3<< 4) | 7), //????
	TVE_IS_480I            =  ((4<< 4) | 7), //???? 
}TVE_VIDEO_FORMAT;

typedef enum _DF_GFX_COLOR_FORMAT_
{
	DF_GFX_CLUT4     = 0,
	DF_GFX_CLUT8     = 1,
	DF_GFX_RGB565    = 2,
	DF_GFX_ARGB4444  = 3,
	DF_GFX_A0        = 4,
	DF_GFX_ARGB1555  = 5,
	DF_GFX_ARGB8888  = 6,
	DF_GFX_FORMA_MAX,

} DF_GFX_COLOR_FORMAT;

typedef enum {
	DF_DEV_NUKOWN = -1,
	DF_DEV_GFX0 = 0,
	DF_DEV_GFX1,
	DF_DEV_VIDEO0,
	DF_DEV_VIDEO1,
	DF_DEV_OUT0,
	DF_DEV_OUT1,
	DF_DEV_TVE0,
	DF_DEV_TVE1,
	DF_DEV_MAX
} DF_DEV_TYPE;

typedef struct DF_DEV_t {
	unsigned int dev_minor;
	DF_DEV_TYPE dev_type;
	spinlock_t spin_lock;
	unsigned int enable;
	TVOUT_MODE tve_format;
	char* name;
} dfdev;

typedef struct _DF_OUTIF_FORMAT_INFO_
{
	int IsYUVorRGB;
	int iWidth;
	int iHeight;
	int iFPS;
	int iFreq;//KHz
	int iIsHD;	
	int iIsInterlaced;
	int iNeedRepeat;
	int iNeedMux;
	int iCVE5F0IsHalfLine;
	int iCVE5F1IsHalfLine;
	int iVgaF0IsHalfLine;
	int iVgaF1IsHalfLIne;
	int iHSyncPolarity;
	int iVSyncPolarity;
	int iDEPolarity;

	unsigned int iXTotal;
	unsigned int iXActiveStart;
	unsigned int iYTotal;
	unsigned int iYTopActiveStart;
	unsigned int iYTopActiveEnd;
	unsigned int iYBotActiveStart;
	unsigned int iYBotActiveEnd;
	unsigned int iCCIRF0Start;
	unsigned int iCCIRF1Start;
	unsigned int iHSyncStart;
	unsigned int iHSyncEnd;
	unsigned int iVSyncTopStart;
	unsigned int iVSyncTopEnd;
	unsigned int iVSyncBotStart;
	unsigned int iVSyncBotEnd;
	int FormatIdx;
	char *NameStr;
}df_out_if_timing;

/*************************IOCTL DEFINE *************************************/
#define CSTVE_IOC_ENABLE	_IOW('e', 0x0b, int)
#define CSTVE_IOC_DISABLE	_IOR('e', 0x0c, int)
#define CSTVE_IOC_SET_MODE	_IOW('e', 0x01, int)
#define CSTVE_IOC_GET_MODE	_IOW('e', 0x02, int)

/* SunHe added to debug brightness, contrast, saturation */
#define CSTVE_IOC_SET_WHILE_LEVEL	 _IOW('e', 0x03, int)
#define CSTVE_IOC_GET_WHILE_LEVEL	 _IOR('e', 0x04, int)
#define CSTVE_IOC_SET_BLACK_LEVEL	 _IOW('e', 0x05, int)
#define CSTVE_IOC_GET_BLACK_LEVEL	 _IOR('e', 0x06, int)
#define CSTVE_IOC_SET_STURATION_LEVEL	 _IOW('e', 0x07, int)
#define CSTVE_IOC_GET_STURATION_LEVEL	 _IOR('e', 0x08, int)
#define CSTVE_IOC_SET_COMP_CHAN         _IOW('e', 0x15, int)

/* dual output */
#define CSTVE_IOC_BIND_GFX		_IOW('e', 0x17, int)
#define CSTVE_IOC_BIND_VID		_IOW('e', 0x18, int)
#define CSTVE_IOC_SET_OUTPUT		_IOW('e', 0x19, int)
#define CSTVE_IOC_GET_BIND_INF		_IOW('e',0x20,int)

#define CSDF_IOC_DISP_ON 		_IOW('x', 0x0F, int)
#define CSDF_IOC_DISP_OFF 		_IOW('x', 0x10, int)
#define CSDF_IOC_DISP_VID_ON 		_IOW('x', 0x11, int)
#define CSDF_IOC_DISP_VID_OFF 		_IOW('x', 0x12, int)
#define CSDF_IOC_DISP_GFX_ON 		_IOW('x', 0x1C, int)
#define CSDF_IOC_DISP_GFX_OFF 		_IOW('x', 0x1D, int)
#define CSDF_IOC_DISP_POS 		_IOW('x', 0x13, struct df_output_pos)
//#define CSDF_IOC_DISP_GET_POS 	_IOW('x', 0x14, int)
#define CSDF_IOC_DISP_VID_ALPHA 	_IOW('x', 0x15, int)
//#define CSDF_IOC_DISP_GET_ALPHA 	_IOW('x', 0x16, int)
#define CSDF_IOC_DISP_VIDFMT 		_IOW('x', 0x18, int)
#define CSDF_IOC_Z_ORDER            	_IOW('x', 0x19, int)
//#define CSDF_IOC_GET_Z_ORDER          _IOW('x', 0x1A, int)
#define CSDF_IOC_DISP_SETBGCOLOR       	_IOW('x', 0x1B, int)
#define CSDF_IOC_DISP_SETMODE       	_IOW('x', 0x1E, int)
//#define CSDF_IOC_DISP_CROP       	_IOW('x', 0x1F, struct df_vid_crop)
#define CSDF_IOC_DISP_HD2SD_ENABLE       _IOW('x', 0x20, int)
#define CSDF_IOC_DISP_HD2SD_CONFIG       _IOW('x', 0x21, int)

#define CSDF_IOC_WSS_CTRL      		_IOW('x', 0x22, int)
#define CSDF_IOC_WSS_SETCONFIG       	_IOW('x', 0x23, int)
#define CSDF_IOC_WSS_SETINFO       	_IOW('x', 0x24, int)
#define CSDF_IOC_TTX_CTRL       	_IOW('x', 0x25, int)
#define CSDF_IOC_TTX_SETCONFIG       	_IOW('x', 0x26, int)
#define CSDF_IOC_TTX_SETINFO       	_IOW('x', 0x27, int)


#ifdef __cplusplus
}
#endif

#endif

