#ifndef __ORIONFB_DEV_H__
#define __ORIONFB_DEV_H__

typedef struct _gfx_colorkey {
	char r_min;
	char r_max;

	char g_min;
	char g_max;

	char b_min;
	char b_max;
}gfx_colorkey;

/* 
 * Z-Order numbers are from hardware, don't redefine them! 
 */
#define Z_ORDER_V0_V1_G         0x0123
#define Z_ORDER_V0_G_V1         0x0213
#define Z_ORDER_V1_V0_G         0x1023
#define Z_ORDER_V1_G_V0         0x2013
#define Z_ORDER_G_V0_V1         0x1203
#define Z_ORDER_G_V1_V0         0x2103

#define FBIO_WAITFORVSYNC       _IOW('F', 0x20, u_int32_t)
#define FBIO_GFX_ON             _IOW('F', 0x21, u_int32_t)
#define FBIO_GFX_ALPHA          _IOW('F', 0x22, u_int32_t)
#define FBIO_Z_ORDER            _IOW('F', 0x50, u_int32_t)
#define FBIO_GFX_FLIP		_IOW('F', 0x51, gfx2d_scalor_params)
#define FBIO_GFX_COLORKEY_ON	_IOW('F', 0x52, u_int32_t)
#define FBIO_GFX_COLORKEY_VAL	_IOW('F', 0x53, gfx_colorkey)
#define FBIO_GFX_BGCOLOR	_IOW('F', 0x54, u_int32_t)

#endif
