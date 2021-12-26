/*******************************************************************************

File name   : blit_ioctl.h

Description : orion blitter driver source file

COPYRIGHT (C) Celestial Semiconductor 2007.

Date               Modification                                     Name
----               ------------                                     ----
14 Nov 2007        Created                                           XM.Chen
*******************************************************************************/


#ifndef	_BLIT_IOCTL_H_
#define _BLIT_IOCTL_H_

#include "blit_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum BLIT_SourceType_e
{
    CSBLIT_SOURCE_TYPE_COLOR,
    CSBLIT_SOURCE_TYPE_BITMAP
} CSBLIT_SourceType_t;

typedef struct CSBLIT_Source_s
{
    CSBLIT_SourceType_t     	Type;
    union {
	    CSGFXOBJ_Color_t   	Color;
            CSGFXOBJ_Bitmap_t  	Bitmap;
    } Data;
    CSGFXOBJ_Rectangle_t    	Rectangle;
    //CSGFXOBJ_Palette_t*     	Palette_p;
    CSGFXOBJ_ColorKeyRGB_t  	ColorKeyRGB;	/* ColorKey default is not used */
    CSGFXOBJ_Color_t	    	DefaultColor; 	/* not valid if source type is CSBLIT_SOURCE_TYPE_COLOR */
    boolean		    	VReverse;		/*added by xm*/
} CSBLIT_Source_t;

typedef struct CSBLIT_Destination_s
{
    CSGFXOBJ_Bitmap_t	      	Bitmap;
    CSGFXOBJ_Rectangle_t	Rectangle;
    //CSGFXOBJ_Palette_t*     	Palette_p;
    boolean		    	VReverse;	/*added by xm*/
} CSBLIT_Destination_t;

typedef struct CSBLIT_FillRectParams_s
{
    CSGFXOBJ_Bitmap_t        	Bitmap;
    CSGFXOBJ_Rectangle_t     	Rectangle;
    CSGFXOBJ_Color_t         	Color;
    CSBLIT_AluMode_t 		AluMode;
} CSBLIT_FillRectParams_t;

typedef struct CSBLIT_CopyRectParams_s
{
    CSGFXOBJ_Bitmap_t        	SrcBitmap;
    CSGFXOBJ_Rectangle_t     	SrcRectangle;
    CSGFXOBJ_Bitmap_t        	DstBitmap;
    s32                   	DstPositionX;
    s32                   	DstPositionY;
    CSBLIT_AluMode_t 		AluMode;
} CSBLIT_CopyRectParams_t;

typedef struct CSBLIT_ScalorParams_s
{
    CSBLIT_Source_t		Src;
    CSBLIT_Destination_t   	Dst;
    u32				VInitialPhase;	/*default is 4*/
    u32				HInitialPhase;	/*default is 0*/
    CSBLIT_AluMode_t 		AluMode;
} CSBLIT_ScalorParams_t;

typedef struct CSBLIT_CompParams_s
{
    boolean			BlendEnable;
    boolean			IsS0OnTopS1;
    u32				ROPAlphaCtrl;
    CSBLIT_Source_t		Src0;
    CSBLIT_Source_t		Src1;
    CSBLIT_Destination_t   	Dst;
    CSBLIT_AluMode_t 		AluMode;
} CSBLIT_CompParams_t;


/* exported functions */
int blit_initialize(CSBlit_HW_Device_t *hBlitDC);

int Fill
( 
    CSBlit_HW_Device_t 		*hBlitDC, 
    s32                 	BlitSrcId,
    CSGFXOBJ_Bitmap_t       	*DesImg, 
    CSGFXOBJ_Rectangle_t    	*DesRect, 
    CSGFXOBJ_Color_t 		*FillColor,
    CSBLIT_AluMode_t 		AluMode
);

int Copy
( 
    CSBlit_HW_Device_t  	*hBlitDC, 
    s32                  	BlitSrcId,
    CSGFXOBJ_Bitmap_t    	*SrcImg, 
    CSGFXOBJ_Rectangle_t 	*SrcRect, 
    s32                  	SrcVReverse,
    CSGFXOBJ_Bitmap_t    	*DesImg, 
    CSGFXOBJ_Rectangle_t 	*DesRect, 
    CSBLIT_AluMode_t 		AluMode,
    s32                  	DesVReverse
);

int Scalor
( 
    CSBlit_HW_Device_t  	*hBlitDC, 
    CSBLIT_Source_t		*SrcPara,    
    CSBLIT_Destination_t 	*DesPara,
    u32                  	VInitialPhase, 
    u32                  	HInitialPhase,
    CSBLIT_AluMode_t 		AluMode
);

int Composite
(
    CSBlit_HW_Device_t 		*hBlitDC, 
    CSBLIT_Source_t 		*Src0Para,
    CSBLIT_Source_t 		*Src1Para, 
    CSBLIT_Destination_t 	*DesPara,
    CSBLIT_AluMode_t 		AluMode,
    boolean 			BlendEnable, 
    boolean 			IsS0OnTopS1, 	
    u32 			ROPAlphaCtrl
);

int CompositeSrc0
(
    CSBlit_HW_Device_t 		*hBlitDC, 
    CSBLIT_Source_t  		*Src0Para,
    CSBLIT_Source_t     	*Src1Para,
    CSBLIT_Destination_t  	*DesPara,
    CSBLIT_AluMode_t 		AluMode,
    boolean 			BlendEnable, 
    boolean 			IsS0OnTopS1, 
    u32 			ROPAlphaCtrl
);

int CompositeSrc1
(
    CSBlit_HW_Device_t 		*hBlitDC, 
    CSBLIT_Source_t     	*Src0Para, 
    CSBLIT_Source_t     	*Src1Para,
    CSBLIT_Destination_t  	*DesPara,
    CSBLIT_AluMode_t 		AluMode,
    boolean 			BlendEnable, 
    boolean 			IsS0OnTopS1, 
    u32 			ROPAlphaCtrl
);

#ifdef __cplusplus
}
#endif

#endif //_BLIT_IOCTL_H_

