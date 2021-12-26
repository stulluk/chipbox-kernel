/*******************************************************************************

File name   : orion_gfxobj.h

Description : 

COPYRIGHT (C) CelestialSemi 2007.

Date               Modification                               Name
----               ------------                               ----
2007-03-30         Created                                    xm.chen
*******************************************************************************/

/* Define to prevent recursive inclusion */

#ifndef __ORION_GFXOBJ_H____
#define __ORION_GFXOBJ_H____

/* Includes ----------------------------------------------------------------- */
#include <linux/types.h>

/* C++ support */
#ifdef __cplusplus
extern "C" {
#endif

/* Exported Constants ------------------------------------------------------- */
typedef u8 boolean;
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

typedef enum CSGFXOBJ_ColorKeyType_e
{
  CSGFXOBJ_COLOR_KEY_TYPE_CLUT1,
  CSGFXOBJ_COLOR_KEY_TYPE_CLUT8,
  CSGFXOBJ_COLOR_KEY_TYPE_RGB888,
  CSGFXOBJ_COLOR_KEY_TYPE_YCbCr888_SIGNED,
  CSGFXOBJ_COLOR_KEY_TYPE_YCbCr888_UNSIGNED,
  CSGFXOBJ_COLOR_KEY_TYPE_RGB565
} CSGFXOBJ_ColorKeyType_t;

typedef enum CSGFXOBJ_ColorType_e
{

  CSGFXOBJ_COLOR_TYPE_RGB565 = 0,
  CSGFXOBJ_COLOR_TYPE_ARGB4444 = 1,
  CSGFXOBJ_COLOR_TYPE_A0 = 2,       /*one single color, can't be set to output colortype*/
  CSGFXOBJ_COLOR_TYPE_ARGB1555 = 3,
  CSGFXOBJ_COLOR_TYPE_CLUT4 = 4,
  CSGFXOBJ_COLOR_TYPE_CLUT8 = 5,
  CSGFXOBJ_COLOR_TYPE_ARGB8888,
  CSGFXOBJ_COLOR_TYPE_RGB888,
  CSGFXOBJ_COLOR_TYPE_ARGB8565,

  CSGFXOBJ_COLOR_TYPE_CLUT2,
  CSGFXOBJ_COLOR_TYPE_CLUT1,
  CSGFXOBJ_COLOR_TYPE_ACLUT88,
  CSGFXOBJ_COLOR_TYPE_ACLUT44,

  CSGFXOBJ_COLOR_TYPE_SIGNED_YCBCR888_444,
  CSGFXOBJ_COLOR_TYPE_UNSIGNED_YCBCR888_444,
  CSGFXOBJ_COLOR_TYPE_SIGNED_YCBCR888_422,
  CSGFXOBJ_COLOR_TYPE_UNSIGNED_YCBCR888_422,
  CSGFXOBJ_COLOR_TYPE_SIGNED_YCBCR888_420,
  CSGFXOBJ_COLOR_TYPE_UNSIGNED_YCBCR888_420,
  CSGFXOBJ_COLOR_TYPE_UNSIGNED_AYCBCR6888_444,
  CSGFXOBJ_COLOR_TYPE_SIGNED_AYCBCR8888,
  CSGFXOBJ_COLOR_TYPE_UNSIGNED_AYCBCR8888,

  CSGFXOBJ_COLOR_TYPE_ALPHA1,
  CSGFXOBJ_COLOR_TYPE_ALPHA4,
  CSGFXOBJ_COLOR_TYPE_ALPHA8,
  CSGFXOBJ_COLOR_TYPE_BYTE,

  CSGFXOBJ_COLOR_TYPE_ARGB8888_255,
  CSGFXOBJ_COLOR_TYPE_ARGB8565_255,
  CSGFXOBJ_COLOR_TYPE_ACLUT88_255,
  CSGFXOBJ_COLOR_TYPE_ALPHA8_255

} CSGFXOBJ_ColorType_t;

typedef enum CSGFXOBJ_PaletteType_e
{
  CSGFXOBJ_PALETTE_TYPE_DEVICE_INDEPENDENT,
  CSGFXOBJ_PALETTE_TYPE_DEVICE_DEPENDENT
}CSGFXOBJ_PaletteType_t;

typedef enum CSGFXOBJ_ScanType_e
{
    CSGFXOBJ_INTERLACED_SCAN,
    CSGFXOBJ_PROGRESSIVE_SCAN    
} CSGFXOBJ_ScanType_t;


/* Exported Types ----------------------------------------------------------- */
typedef struct CSGFXOBJ_Bitmap_s
{
  CSGFXOBJ_ColorType_t			ColorType;
  u32                                   Width;
  u32                                   Height;
  u32                                   Pitch;
  u32                                   Offset;
  void*                                 Data_p;
  u32                                   Size;
  boolean				NibbleLittleNotBigEndian;
  boolean				ByteLittleNotBigEndian;
  boolean				TwoBytesLittleNotBigEndian;  
} CSGFXOBJ_Bitmap_t;

typedef struct CSGFXOBJ_ColorACLUT_s
{
  u8 Alpha;
  u8 PaletteEntry;
} CSGFXOBJ_ColorACLUT_t;

typedef struct CSGFXOBJ_ColorARGB_s
{
  u8 Alpha;
  u8 R;
  u8 G;
  u8 B;
} CSGFXOBJ_ColorARGB_t;

typedef struct CSGFXOBJ_ColorKeyCLUT_s
{
  u8      PaletteEntryMin;
  u8      PaletteEntryMax;
  boolean    PaletteEntryOut;
  boolean    PaletteEntryEnable;
} CSGFXOBJ_ColorKeyCLUT_t;

typedef struct CSGFXOBJ_ColorKeyRGB_s
{
  u8      RMin;
  u8      RMax;
  boolean    ROut;
  
  u8      GMin;
  u8      GMax;
  boolean    GOut;

  u8      BMin;
  u8      BMax;
  boolean    BOut;

  boolean    RGBEnable;
} CSGFXOBJ_ColorKeyRGB_t;

typedef union CSGFXOBJ_ColorKeyValue_u
{
  CSGFXOBJ_ColorKeyCLUT_t           CLUT1;
  CSGFXOBJ_ColorKeyCLUT_t           CLUT8;
  CSGFXOBJ_ColorKeyRGB_t            RGB888;
  CSGFXOBJ_ColorKeyRGB_t            RGB565;
} CSGFXOBJ_ColorKeyValue_t;

typedef struct CSGFXOBJ_ColorKey_s
{
  CSGFXOBJ_ColorKeyType_t           Type;
  CSGFXOBJ_ColorKeyValue_t          Value;
} CSGFXOBJ_ColorKey_t;

typedef struct CSGFXOBJ_ColorRGB_s
{
  u8 R;
  u8 G;
  u8 B;
} CSGFXOBJ_ColorRGB_t;

typedef union CSGFXOBJ_ColorValue_u
{
  CSGFXOBJ_ColorARGB_t           ARGB8888;
  CSGFXOBJ_ColorRGB_t            RGB888;
  CSGFXOBJ_ColorARGB_t           ARGB8565;
  CSGFXOBJ_ColorRGB_t            RGB565;
  CSGFXOBJ_ColorARGB_t           ARGB1555;
  CSGFXOBJ_ColorARGB_t           ARGB4444;

  u8                            CLUT8;
  u8                            CLUT4;
  u8                            CLUT2;
  u8                            CLUT1;
  CSGFXOBJ_ColorACLUT_t          ACLUT88 ;
  CSGFXOBJ_ColorACLUT_t          ACLUT44 ;

  u8                            ALPHA1;
  u8                            ALPHA4;
  u8                            ALPHA8;
  u8                            Byte;

} CSGFXOBJ_ColorValue_t;

typedef struct CSGFXOBJ_Color_s
{
  CSGFXOBJ_ColorType_t            Type;
  CSGFXOBJ_ColorValue_t           Value;
} CSGFXOBJ_Color_t;

typedef struct CSGFXOBJ_Palette_s
{
  CSGFXOBJ_ColorType_t       	ColorType;
  CSGFXOBJ_PaletteType_t     	PaletteType;
  u8                         	ColorDepth;
  void*                     	Data_p;

} CSGFXOBJ_Palette_t;

typedef struct CSGFXOBJ_Rectangle_s
{
  s32 PositionX;
  s32 PositionY;
  u32 Width;
  u32 Height;
} CSGFXOBJ_Rectangle_t;

/* C++ support */
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __ORION_GFXOBJ_H */

/* End of orion_gxobj.h */

