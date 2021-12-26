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
#include "orionfb_1200_hw.h"

static int update_gfx_layer(struct fb_info *info, int init);

static volatile unsigned long  *df_base = NULL;

#define _GFX_EN		0
#define _GFX_ALPHA	1
#define _GFX_BUF_START	2
#define _GFX_LINE_PITCH	3
#define _GFX_X_START	4
#define _GFX_Y_START	5
#define _GFX_X_END	6
#define _GFX_Y_END	7
#define _GFX_FORMAT	8
#define _GFX_KEYING_EN	9
#define _GFX_R_MIN	10
#define _GFX_R_MAX	11
#define _GFX_G_MIN	12
#define _GFX_G_MAX	13
#define _GFX_B_MIN	14
#define _GFX_B_MAX	15

static int gfx_regs_lst[2][16] = {
	{
		DF_GFX1_EN,
		DF_GFX1_DEFAULT_ALPHA,
		DF_GFX1_BUF_START,
		DF_GFX1_LINE_PITCH,
		DF_GFX1_X_START,
		DF_GFX1_Y_START,
		DF_GFX1_X_END,
		DF_GFX1_Y_END,
		DF_GFX1_FORMAT,
		DF_GFX1_KEYING_EN,
		DF_GFX1_R_MIN,
		DF_GFX1_R_MAX,
		DF_GFX1_G_MIN,
		DF_GFX1_G_MAX,
		DF_GFX1_B_MIN,
		DF_GFX1_B_MAX
	},
	{
		DF_GFX2_EN,
		DF_GFX2_DEFAULT_ALPHA,
		DF_GFX2_BUF_START,
		DF_GFX2_LINE_PITCH,
		DF_GFX2_X_START,
		DF_GFX2_Y_START,
		DF_GFX2_X_END,
		DF_GFX2_Y_END,
		DF_GFX2_FORMAT,
		DF_GFX2_KEYING_EN,
		DF_GFX2_R_MIN,
		DF_GFX2_R_MAX,
		DF_GFX2_G_MIN,
		DF_GFX2_G_MAX,
		DF_GFX2_B_MIN,
		DF_GFX2_B_MAX
	}
};

int orionfb_hw_init(struct fb_info *info, int en_flags)
{
	if (NULL == df_base)
		if(!(df_base = (unsigned long *)ioremap(DF_REG_BASE, DF_REG_SIZE))) return -ENXIO;

	update_gfx_layer(info, en_flags);

	return 0;
}

void orionfb_hw_exit(void)
{
	iounmap((void *)df_base);
}

int orionfb_hw_waitforvsync(struct fb_info *info)
{
	/* TODO: to call an external functions from video modules */
	
	return 0;
}

int orionfb_hw_gfx_on(struct fb_info *info, unsigned int arg)
{
	int i_node = info->node;
	if (i_node > 1) i_node -= 2;

	df_base[DF_UPDATE_REG] = 0x1;
	df_base[gfx_regs_lst[i_node][_GFX_EN]] = arg ? 1 : 0;
	df_base[DF_UPDATE_REG] = 0x0;

        return 0;
}

int orionfb_hw_gfx_alpha(struct fb_info *info, unsigned int arg)
{
	int i_node = info->node;
	if (i_node > 1) i_node -= 2;

        df_base[DF_UPDATE_REG] = 0x1;
	df_base[gfx_regs_lst[i_node][_GFX_ALPHA]] = arg & 0xff;
        df_base[DF_UPDATE_REG] = 0x0;

        return 0;
}

int orionfb_hw_colorkey_on(struct fb_info *info, unsigned int arg)
{
	int i_node = info->node;
	if (i_node > 1) i_node -= 2;

        df_base[DF_UPDATE_REG] = 0x1;
	df_base[gfx_regs_lst[i_node][_GFX_KEYING_EN]] = arg;
        df_base[DF_UPDATE_REG] = 0x0;

	return 0;
}

int orionfb_hw_colorkey_val(struct fb_info *info, gfx_colorkey *col_key)
{
	int i_node = info->node;
	if (i_node > 1) i_node -= 2;

        df_base[DF_UPDATE_REG] = 0x1;

	df_base[gfx_regs_lst[i_node][_GFX_R_MIN]] = col_key->r_min;
	df_base[gfx_regs_lst[i_node][_GFX_R_MAX]] = col_key->r_max;
	df_base[gfx_regs_lst[i_node][_GFX_G_MIN]] = col_key->g_min;
	df_base[gfx_regs_lst[i_node][_GFX_G_MAX]] = col_key->g_max;
	df_base[gfx_regs_lst[i_node][_GFX_B_MIN]] = col_key->b_min;
	df_base[gfx_regs_lst[i_node][_GFX_B_MAX]] = col_key->b_max;

        df_base[DF_UPDATE_REG] = 0x0;

	return 0;
}

int orionfb_hw_z_order(struct fb_info *info, unsigned int arg)
{
    df_base[DF_UPDATE_REG]      = 0x1;

    df_base[DF_VIDEO_Z_ORDER]   = (arg >> 12) & 0xf;
    df_base[DF_VIDEO2_Z_ORDER]  = (arg >>  8) & 0xf;
    df_base[DF_GFX1_Z_ORDER]    = (arg >>  4) & 0xf;
    df_base[DF_GFX2_Z_ORDER]    = (arg >>  0) & 0xf;

    df_base[DF_UPDATE_REG]      = 0x0;

    return 0;
}

int orionfb_hw_setbgcolor(unsigned int arg)
{
    df_base[DF_UPDATE_REG]      = 0x1;

    df_base[DF_BG_Y]   = (arg >> 16) & 0xff;
    df_base[DF_BG_U]   = (arg >> 8) & 0xff;
    df_base[DF_BG_V]   = (arg >> 0) & 0xff;

    df_base[DF_UPDATE_REG]      = 0x0;

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

static int update_gfx_layer(struct fb_info *info, int init)
{
	//if (info->node > 1) return 0; /* fb2 and fb3 are virtual framebuffer */

	df_base[DF_UPDATE_REG] = 0x1;

	/* if info->node > 1,  only update gfx color format */
	if (info->node > 1) {	
	    df_base[gfx_regs_lst[info->node - 2][_GFX_FORMAT]] &= ~0x00000007; 
	    df_base[gfx_regs_lst[info->node - 2][_GFX_FORMAT]] |= 
		(info->var.bits_per_pixel == 8) ? GFX_FMT_CLUT8 :
		(info->var.transp.length == 0) ? GFX_FMT_RGB565 :
		(info->var.transp.length == 1) ? GFX_FMT_ARGB1555 :
		(info->var.transp.length == 4) ? GFX_FMT_ARGB4444 :
		df_base[gfx_regs_lst[info->node][_GFX_FORMAT]];

	    df_base[DF_UPDATE_REG] = 0x0;
	    return 0;
	}

#ifdef CONFIG_ARCH_ORION_CSM1200
	df_base[DF_BYTE_ENDIAN]	= 0x3;
#elif defined(CONFIG_ARCH_ORION_CSM1100)
	df_base[DF_BYTE_ENDIAN]	= 0x0;
#endif

	df_base[gfx_regs_lst[info->node][_GFX_BUF_START]] = 
		(info->fix.smem_start +
		info->fix.line_length * info->var.yoffset +
		info->var.xoffset * info->var.bits_per_pixel / 8) >> 4;

	df_base[gfx_regs_lst[info->node][_GFX_LINE_PITCH]] = info->fix.line_length >> 4;
	df_base[gfx_regs_lst[info->node][_GFX_X_START]] = info->var.left_margin;

	df_base[gfx_regs_lst[info->node][_GFX_Y_START]]	= 
		info->var.upper_margin / 
		(info->var.vmode & FB_VMODE_INTERLACED ? 2 : 1);

	df_base[gfx_regs_lst[info->node][_GFX_X_END]] = (info->var.left_margin + info->var.xres) - 1;  

	df_base[gfx_regs_lst[info->node][_GFX_Y_END]] = 
		(info->var.upper_margin + info->var.yres) /
		(info->var.vmode & FB_VMODE_INTERLACED ? 2 : 1);

	df_base[gfx_regs_lst[info->node][_GFX_FORMAT]] &= ~0x00000007; 
	df_base[gfx_regs_lst[info->node][_GFX_FORMAT]] |= 
		(info->var.bits_per_pixel == 8) ? GFX_FMT_CLUT8 :
		(info->var.transp.length == 0) ? GFX_FMT_RGB565 :
		(info->var.transp.length == 1) ? GFX_FMT_ARGB1555 :
		(info->var.transp.length == 4) ? GFX_FMT_ARGB4444 :
		df_base[gfx_regs_lst[info->node][_GFX_FORMAT]];

	if(init) {
		df_base[gfx_regs_lst[info->node][_GFX_ALPHA]] = 0xff;
		df_base[gfx_regs_lst[info->node][_GFX_EN]] = 0x1;

		df_base[gfx_regs_lst[info->node][_GFX_KEYING_EN]] = 1;

		df_base[gfx_regs_lst[info->node][_GFX_R_MIN]] = 0x00;
		df_base[gfx_regs_lst[info->node][_GFX_R_MAX]] = 0x00;
		df_base[gfx_regs_lst[info->node][_GFX_G_MIN]] = 0x00;
		df_base[gfx_regs_lst[info->node][_GFX_G_MAX]] = 0x00;
		df_base[gfx_regs_lst[info->node][_GFX_B_MIN]] = 0x00;
		df_base[gfx_regs_lst[info->node][_GFX_B_MAX]] = 0x00;

		/* the following action is for display color correction. */
		df_base[0x0e8>>2] = 0x01;
		df_base[0x18c>>2] = 0xfc;
		df_base[0x188>>2] = 0x01;
		df_base[0x190>>2] = 0xff;
	}

	df_base[DF_UPDATE_REG] = 0x0;

	return 0;	
}

void df_writel(int addr, int val)
{ 
	df_base[addr >> 2] = val;
}

int df_readl(int addr)
{
	return  df_base[addr >> 2];
}

