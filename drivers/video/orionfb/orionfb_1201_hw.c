#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/fb.h>
#include <linux/init.h>

#include "orionfb_dev.h"
#include "df_reg_def.h"
#include "df_reg_fmt.h"

struct orionfb_interrupt {
	wait_queue_head_t wait;
	unsigned int count;
	int is_display;
	struct fb_info *cur_fb_info;
};
struct orionfb_interrupt oriondf_1201_vblank_int[2];

extern int __Is_TV;
extern df_reg_para dfreg;
extern void __df_update_start(void);
extern void __df_update_end(void);
extern unsigned int DF_Read(unsigned int addr);
extern void DF_Write(unsigned int addr, unsigned int data);
extern void DFGfxEna(int GfxId, int IsEna, int IsRGB2YUVEna);
extern void DFGfxSetAlpha(int GfxId, unsigned char DefaultAlpha, unsigned char ARGB1555Alpha0, unsigned char ARGB1555Alpha1);
extern void DFGfxColorKeyEna(int GfxId,int IsEna);
extern void DFGfxSetColorKey(int GfxId, unsigned char KeyRMin, unsigned char KeyRMax, unsigned char KeyGMin, unsigned char KeyGMax, unsigned char KeyBMin, unsigned char KeyBMax);
extern void DFSetZOrder(int OutIFId, int Gfx1ZOrder, int Gfx2ZOrder, int Video1ZOrder, int Video2ZOrder);
extern int Get_Outif_Id(int gfx_id);

#define DF_GFX_TRUE_COLOR_SUPPORT

typedef enum _DF_GFX_COLOR_FORMAT_
{
	DF_GFX_CLUT4     = 0,
	DF_GFX_CLUT8     = 1,
	DF_GFX_RGB565    = 2,
	DF_GFX_ARGB4444  = 3,
	DF_GFX_A0        = 4,
	DF_GFX_ARGB1555  = 5,
#ifdef DF_GFX_TRUE_COLOR_SUPPORT
	DF_GFX_ARGB8888  = 6,
#endif
	DF_GFX_FORMA_MAX,

} DF_GFX_COLOR_FORMAT;

static int dfreg_gfx[2][13] =
{
	{DISP_GFX1_CTRL,	//=0
	DISP_GFX1_FORMAT,	//=1
	DISP_GFX1_ALPHA_CTRL,	//=2
	DISP_GFX1_KEY_RED,	//=3
	DISP_GFX1_KEY_BLUE,	//=4
	DISP_GFX1_KEY_GREEN,	//=5
	DISP_GFX1_BUF_START,	//=6
	DISP_GFX1_LINE_PITCH,	//=7
	DISP_GFX1_X_POSITON,	//=8
	DISP_GFX1_Y_POSITON,	//=9
	DISP_GFX1_SCL_X_POSITON,	//=10
	DISP_GFX1_CLUT_ADDR,	//=11
	DISP_GFX1_CLUT_DATA	//=12
	},
	{DISP_GFX2_CTRL,
	DISP_GFX2_FORMAT,
	DISP_GFX2_ALPHA_CTRL,
	DISP_GFX2_KEY_RED,
	DISP_GFX2_KEY_BLUE,
	DISP_GFX2_KEY_GREEN,
	DISP_GFX2_BUF_START,
	DISP_GFX2_LINE_PITCH,
	DISP_GFX2_X_POSITON,
	DISP_GFX2_Y_POSITON,
	DISP_GFX2_SCL_X_POSITON,
	DISP_GFX2_CLUT_ADDR,
	DISP_GFX2_CLUT_DATA
	},
};

/************************************* FUNCTION *****************************************/
int update_gfx_layer(struct fb_info *info, int init)
{
	unsigned int outif_id = 0;
	unsigned int val = 0;
	int i_node = info->node;

	if (i_node > 1) i_node -= 2;
	outif_id = Get_Outif_Id(i_node);

	//if (info->node > 1) return 0; /* fb2 and fb3 are virtual framebuffer */
	__df_update_start();

	/* if info->node > 1,  only update gfx color format */
	if (info->node > 1) {
		dfreg.Gfx[info->node -2].df_gfx_format_reg.val = DF_Read(dfreg_gfx[info->node -2][1]);
		dfreg.Gfx[info->node -2].df_gfx_line_pitch_reg.val = DF_Read(dfreg_gfx[info->node -2][7]);

		dfreg.Gfx[info->node -2].df_gfx_line_pitch_reg.bits.iLinePitch = info->fix.line_length >> 4;
		dfreg.Gfx[info->node - 2].df_gfx_format_reg.bits.iColorFormat =
			(info->var.bits_per_pixel == 4) ? DF_GFX_CLUT4 :
			(info->var.bits_per_pixel == 8) ? DF_GFX_CLUT8 :
			(info->var.transp.length == 0) ? DF_GFX_RGB565 :
			(info->var.transp.length == 1) ? DF_GFX_ARGB1555 :
			(info->var.transp.length == 4) ? DF_GFX_ARGB4444 :
#ifdef DF_GFX_TRUE_COLOR_SUPPORT
			(info->var.transp.length == 8) ? DF_GFX_ARGB8888 :
			(info->var.transp.length == 32) ? DF_GFX_A0 :
#else
			(info->var.transp.length == 16) ? DF_GFX_A0 :
#endif
			dfreg.Gfx[info->node -2].df_gfx_format_reg.bits.iColorFormat;

		if(dfreg.Gfx[info->node -2].df_gfx_format_reg.bits.iColorFormat == DF_GFX_ARGB8888)
			dfreg.Gfx[info->node -2].df_gfx_format_reg.bits.i16BitEndian = 1;
		else
			dfreg.Gfx[info->node -2].df_gfx_format_reg.bits.i16BitEndian = 0;

		dfreg.Gfx[info->node -2].df_gfx_format_reg.bits.i128BitEndian = 1;
		dfreg.Gfx[info->node -2].df_gfx_format_reg.bits.iByteEndian = 1;

		DF_Write((dfreg_gfx[info->node -2][1]), dfreg.Gfx[info->node -2].df_gfx_format_reg.val);
		DF_Write((dfreg_gfx[info->node -2][7]), dfreg.Gfx[info->node -2].df_gfx_line_pitch_reg.val);

		__df_update_end();
		return 0;
	}

#if 1
	dfreg.Gfx[info->node].df_gfx_format_reg.val = DF_Read(dfreg_gfx[info->node][1]);
	dfreg.Gfx[info->node].df_gfx_buf_start_addr_reg.val = DF_Read(dfreg_gfx[info->node][6]);
	dfreg.Gfx[info->node].df_gfx_line_pitch_reg.val = DF_Read(dfreg_gfx[info->node][7]);
	dfreg.Gfx[info->node].df_gfx_x_position_reg.val = DF_Read(dfreg_gfx[info->node][8]);
	dfreg.Gfx[info->node].df_gfx_y_position_reg.val = DF_Read(dfreg_gfx[info->node][9]);
	dfreg.Gfx[info->node].df_gfx_scl_x_position_reg.val = DF_Read(dfreg_gfx[info->node][10]);
#endif

#ifdef CONFIG_ARCH_ORION_CSM1200
	df_base[DF_BYTE_ENDIAN]	= 0x3;
#elif defined(CONFIG_ARCH_ORION_CSM1100)
	df_base[DF_BYTE_ENDIAN]	= 0x0;
#elif defined(CONFIG_ARCH_ORION_CSM1201)
	if(dfreg.Gfx[info->node].df_gfx_format_reg.bits.iColorFormat == DF_GFX_ARGB8888)
		dfreg.Gfx[info->node].df_gfx_format_reg.bits.i16BitEndian = 1;
	else
		dfreg.Gfx[info->node].df_gfx_format_reg.bits.i16BitEndian = 0;
	
	dfreg.Gfx[info->node].df_gfx_format_reg.bits.i128BitEndian = 1;
	dfreg.Gfx[info->node].df_gfx_format_reg.bits.iByteEndian = 1;
#endif
	dfreg.Gfx[info->node].df_gfx_buf_start_addr_reg.bits.iStartAddr =
	(info->fix.smem_start +
	info->fix.line_length * info->var.yoffset +
	info->var.xoffset * info->var.bits_per_pixel / 8) >> 4;

	dfreg.Gfx[info->node].df_gfx_line_pitch_reg.bits.iLinePitch = info->fix.line_length >> 4;
	dfreg.Gfx[info->node].df_gfx_line_pitch_reg.bits.iBlankPixel = 0;

	dfreg.Gfx[info->node].df_gfx_x_position_reg.bits.iXStart = info->var.left_margin;
	dfreg.Gfx[info->node].df_gfx_x_position_reg.bits.iXEnd = (info->var.left_margin + info->var.xres) - 1;  
	dfreg.Gfx[info->node].df_gfx_y_position_reg.bits.iYStart	= 
		info->var.upper_margin / (info->var.vmode & FB_VMODE_INTERLACED ? 2 : 1);
	dfreg.Gfx[info->node].df_gfx_y_position_reg.bits.iYEnd = 
		(info->var.upper_margin + info->var.yres) /(info->var.vmode & FB_VMODE_INTERLACED ? 2 : 1);

	dfreg.Gfx[info->node].df_gfx_format_reg.bits.iColorFormat =
		(info->var.bits_per_pixel == 4) ? DF_GFX_CLUT4 :
		(info->var.bits_per_pixel == 8) ? DF_GFX_CLUT8 :
		(info->var.transp.length == 0) ? DF_GFX_RGB565 :
		(info->var.transp.length == 1) ? DF_GFX_ARGB1555 :
		(info->var.transp.length == 4) ? DF_GFX_ARGB4444 :
#ifdef DF_GFX_TRUE_COLOR_SUPPORT
		(info->var.transp.length == 8) ? DF_GFX_ARGB8888 :
		(info->var.transp.length == 32) ? DF_GFX_A0 :
#else
		(info->var.transp.length == 16) ? DF_GFX_A0 :
#endif
		dfreg.Gfx[info->node].df_gfx_format_reg.bits.iColorFormat;
	
	if(init) {
		DFGfxSetAlpha(info->node, 0xff, 0, 0);
		DFGfxEna(info->node, 1, __Is_TV);
		DFGfxSetColorKey(info->node, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
		DFGfxColorKeyEna(info->node, 1);
	}

	DF_Write((dfreg_gfx[info->node][6]), dfreg.Gfx[info->node].df_gfx_buf_start_addr_reg.val);
	DF_Write((dfreg_gfx[info->node][7]), dfreg.Gfx[info->node].df_gfx_line_pitch_reg.val);
	DF_Write((dfreg_gfx[info->node][8]), dfreg.Gfx[info->node].df_gfx_x_position_reg.val);
	DF_Write((dfreg_gfx[info->node][9]), dfreg.Gfx[info->node].df_gfx_y_position_reg.val);
	DF_Write((dfreg_gfx[info->node][1]), dfreg.Gfx[info->node].df_gfx_format_reg.val);

	__df_update_end();

	if(oriondf_1201_vblank_int[outif_id].is_display == 1){
		wait_event_interruptible(oriondf_1201_vblank_int[outif_id].wait, oriondf_1201_vblank_int[outif_id].is_display == 0);
		return 0;
	}

	return 0;	
}

int orionfb_hw_init(struct fb_info *info, int en_flags)
{
	int i_node = info->node;
	if (i_node > 1) i_node -= 2;

	init_waitqueue_head(&oriondf_1201_vblank_int[i_node].wait);
	oriondf_1201_vblank_int[i_node].is_display = 0;
	oriondf_1201_vblank_int[i_node].count = 0;
	oriondf_1201_vblank_int[i_node].cur_fb_info = info;
	update_gfx_layer(info, en_flags);

	return 0;
}

void orionfb_hw_exit(void)
{
	;
}

int orionfb_hw_gfx_on(struct fb_info *info, unsigned int arg)
{
	int i_node = info->node;
	if (i_node > 1) i_node -= 2;

	__df_update_start();
	DFGfxEna(i_node, arg, __Is_TV);
	__df_update_end();

	return 0;
}

int orionfb_hw_gfx_alpha(struct fb_info *info, unsigned int arg)
{
	int i_node = info->node;
	if (i_node > 1) i_node -= 2;

	arg &= 0xff;

	__df_update_start();
	DFGfxSetAlpha(i_node, arg, 0, 0xff);
	__df_update_end();

	return 0;
}

int orionfb_hw_colorkey_on(struct fb_info *info, unsigned int arg)
{
	int i_node = info->node;
	if (i_node > 1) i_node -= 2;

	__df_update_start();
	DFGfxColorKeyEna(i_node, arg);
	__df_update_end();

	return 0;
}

int orionfb_hw_colorkey_val(struct fb_info *info, gfx_colorkey *col_key)
{
	int i_node = info->node;
	if (i_node > 1) i_node -= 2;

	__df_update_start();
	DFGfxSetColorKey(i_node, col_key->r_min, col_key->r_max, col_key->g_min, col_key->g_max, col_key->b_min, col_key->b_max);
	__df_update_end();

	return 0;
}

int orionfb_hw_z_order(struct fb_info *info, unsigned int arg)
{
	int Video1ZOrder = (arg >>  0) & 0x3;
	int Video2ZOrder = (arg >>  4) & 0x3;
	int Gfx1ZOrder = (arg >> 8) & 0x3;
	int Gfx2ZOrder = (arg >> 12) & 0x3;
	int i_node = info->node;
	if (i_node > 1) i_node -= 2;

	__df_update_start();
	DFSetZOrder(i_node, Gfx1ZOrder, Gfx2ZOrder, Video1ZOrder, Video2ZOrder);
	__df_update_end();

	return 0;
}

int orionfb_hw_set_par(struct fb_info *info)
{
	return update_gfx_layer(info, 0);
}

int orionfb_hw_pan_display(struct fb_info *info)
{
	return update_gfx_layer(info, 0);
}

int orionfb_hw_waitforvsync(struct fb_info *info)
{
	unsigned int val = 0;
	unsigned int sys_bottom_flag, sys_frame_sync,sys_vsync,sys_line_count;
	unsigned int y_total_line = 0;
	unsigned int outif_id = 0;
	int i_node = info->node;
	
	if (i_node > 1) i_node -= 2;
	outif_id = Get_Outif_Id(i_node);
#if 0
	if(outif_id){
		y_total_line = DF_Read(DISP_OUTIF2_Y_TOTAL);
	}
	else{
		y_total_line = DF_Read(DISP_OUTIF1_Y_TOTAL);
	}

	while(1){
		val = DF_Read(outif_id ? DISP_OUTIF2_STA_LINE : DISP_OUTIF1_STA_LINE);
		sys_line_count = (val >> 16) & 0xfff;
		if(y_total_line - sys_line_count < 10){
			break;
		}
	}
#else
	oriondf_1201_vblank_int[outif_id].is_display = 1;
#endif

	return 0;
}

int orionfb_hw_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
                         u_int transp, struct fb_info *info)
{
	/* TODO: implement CLUT table updating */
	return 0;
}

int orionfb_hw_blank(int blank)
{
	/* TODO: to call an external functions from TVE modules */
	return 0;
}

void df_writel(int addr, int val)
{
	DF_Write(addr, val);
}

int df_readl(int addr)
{
	return DF_Read(addr);
}
