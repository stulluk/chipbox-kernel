/*
 *  linux/drivers/video/orionfb.c -- ORION frame buffer device
 *
 *  Copyright (C) 2007 Celestial Semiconductor
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive for
 *  more details.
 */

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
#include <linux/proc_fs.h>

#include <linux/mem_define.h>

#include "orionfb_dev.h"
#include "orionfb_hw.h"

#include "orionfb_2d.c"

#define FB_ACCEL_ORION_2DGFX	0xff

#define MODULE_NAME 	"orionfb"

#define FB_NUMS		4

/* There are FB_NUMS framebuffers, each represented by an fb_info. */
#define GFX0_FBNAME	"gfx0"
#define GFX1_FBNAME	"gfx1"
#define GFX2_FBNAME	"gfx2"
#define GFX3_FBNAME	"gfx3"

#define MEMIO_START 	0x40000000
#define MEMIO_SIZE	0x1000

static struct orion_info 
{
	struct {		
		struct fb_info *info;

		/* framebuffer area */
		unsigned long fb_base_phys;
		unsigned long fb_base;
		unsigned long fb_size;

		/* to map the registers */
		dma_addr_t mmio_base_phys;
		unsigned long mmio_base;
		unsigned long mmio_size;
	} fbs_lst[FB_NUMS];

	wait_queue_head_t vsync_wait;
	unsigned long vsync_cnt;
	int timeout;

	struct device *dev;
} orion_static;

static struct orion_info *orion = &orion_static;

static struct fb_var_screeninfo orionfb_default __initdata = {
	.xres =         720,
	.yres =         576,
	.xres_virtual = 720,
	.yres_virtual = 576,
	.bits_per_pixel = 16,
	.transp =		{ 0, 0, 0 },
	.red =          { 11, 5, 0 },
	.green =        {  5, 6, 0 },
	.blue =         {  0, 5, 0 },
	.height =       -1,
	.width =        -1,
	.pixclock =     74074,
	.left_margin =  0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len =    144,
	.vsync_len =    49,
	.vmode =        FB_VMODE_INTERLACED,
};
 
static struct fb_fix_screeninfo orionfb_fix __initdata = {
	.id =           "Orion fb",
	.smem_start =   FB0_REGION,
	.smem_len   =   FB0_SIZE,
	.type =         FB_TYPE_PACKED_PIXELS,
	.visual =       FB_VISUAL_TRUECOLOR,
	.xpanstep =     32,
	.ypanstep =     1,
	.ywrapstep =    0,
	.line_length =  1440,
	.mmio_start  =  0x40000000,
	.mmio_len    =  0x1000,
	.accel =        FB_ACCEL_ORION_2DGFX,
};

static struct fb_fix_screeninfo orionfb_fix2 __initdata = {
	.id =           "Orion fb",
	.smem_start =   FB1_REGION,
	.smem_len   =   FB1_SIZE,
	.type =         FB_TYPE_PACKED_PIXELS,
	.visual =       FB_VISUAL_TRUECOLOR,
	.xpanstep =     32,
	.ypanstep =     1,
	.ywrapstep =    0,
	.line_length =  1440,
	.mmio_start  =  0x40000000,
	.mmio_len    =  0x1000,
	.accel =        FB_ACCEL_ORION_2DGFX,
};

static struct fb_fix_screeninfo orionfb_fix3 __initdata = {
	.id =           "Orion fb",
	.smem_start =   FB2_REGION,
	.smem_len   =   FB2_SIZE,
	.type =         FB_TYPE_PACKED_PIXELS,
	.visual =       FB_VISUAL_TRUECOLOR,
	.xpanstep =     32,
	.ypanstep =     1,
	.ywrapstep =    0,
	.line_length =  1440,
	.mmio_start  =  0x40000000,
	.mmio_len    =  0x1000,
	.accel =        FB_ACCEL_ORION_2DGFX,
};

static struct fb_fix_screeninfo orionfb_fix4 __initdata = {
	.id =           "Orion fb",
	.smem_start =   FB3_REGION,
	.smem_len   =   FB3_SIZE,
	.type =         FB_TYPE_PACKED_PIXELS,
	.visual =       FB_VISUAL_TRUECOLOR,
	.xpanstep =     32,
	.ypanstep =     1,
	.ywrapstep =    0,
	.line_length =  1440,
	.mmio_start  =  0x40000000,
	.mmio_len    =  0x1000,
	.accel =        FB_ACCEL_ORION_2DGFX,
};

static struct res_i {
	u_long xres;
	u_long yres;
} res_t[] = {
	{720,	576},
	{720,	480},
	{1280,	720},
	{1920,	1080},
	{640,	480},
	{800,	600},
	{1024,	768},
	{1280,	1024},
	{1600,	1000},
	{1280,	720},
	{848,	480},
	{800,	480}};

static int orionfb_check_var(struct fb_var_screeninfo *var,
		struct fb_info *info);
static int orionfb_set_par(struct fb_info *info);
static int orionfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
		u_int transp, struct fb_info *info);
static int orionfb_pan_display(struct fb_var_screeninfo *var,
		struct fb_info *info);
static int orionfb_blank(int blank, struct fb_info *info);
static int orionfb_ioctl(struct inode *inode, struct file *file, u_int cmd,
		u_long arg, struct fb_info *info);


static struct fb_ops orionfb_ops = {
	.fb_check_var   = orionfb_check_var,
	.fb_set_par     = orionfb_set_par,
	.fb_setcolreg   = orionfb_setcolreg,
	.fb_pan_display = orionfb_pan_display,
	.fb_blank       = orionfb_blank,
	.fb_ioctl       = orionfb_ioctl,

	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
	.fb_cursor      = soft_cursor
};

/*
 * Internal routines
 */
static u_long get_line_length(int xres_virtual, int bpp)
{
	u_long length;

	length = xres_virtual * bpp;
	length = (length + 127) & ~127;
	length >>= 3;

	return length;
}

/*
 * Setting the video mode has been split into two parts.
 * First part, xxxfb_check_var, must not write anything
 * to hardware, it should only verify and adjust var.
 * This means it doesn't alter par but it does use hardware
 * data from it to check this var. 
 */
static int orionfb_check_var(struct fb_var_screeninfo *var,
		struct fb_info *info)
{
	u_long i;
	u_long line_length;
	u_long valid_res = 0;

	/*
	 *  FB_VMODE_CONUPDATE and FB_VMODE_SMOOTH_XPAN are equal!
	 *  as FB_VMODE_SMOOTH_XPAN is only used internally
	 */
	if (var->vmode & FB_VMODE_CONUPDATE) {
		var->vmode |= FB_VMODE_YWRAP;
		var->xoffset = info->var.xoffset;
		var->yoffset = info->var.yoffset;
	}

	/* minimal resolution */
	if (!var->xres)
		var->xres = 32;
	if (!var->yres)
		var->yres = 2;

	/* validate resolutions */
	for(i = 0; i < sizeof(res_t)/sizeof(struct res_i); i++) {
		if(res_t[i].yres == (var->upper_margin + var->yres + var->lower_margin) &&
				res_t[i].xres == (var->left_margin  + var->xres + var->right_margin )) {
			valid_res = 1;
			break;
		}
	}

	if(!valid_res)
	{
		return -EINVAL;
	}

	if (var->xres > var->xres_virtual)
		var->xres_virtual = var->xres;
	if (var->yres > var->yres_virtual)
		var->yres_virtual = var->yres;
	if (var->bits_per_pixel <= 4)
		var->bits_per_pixel = 4;
	else if (var->bits_per_pixel <= 8)
		var->bits_per_pixel = 8;
	else if (var->bits_per_pixel <= 16)
		var->bits_per_pixel = 16;
#if defined(CONFIG_ARCH_ORION_CSM1201)
	else if (var->bits_per_pixel <= 32)
		var->bits_per_pixel = 32;
#endif
	else
	{
		return -EINVAL;
	}

	/* width must be 16byte aligned */
	if(var->xres % (16 * 8 / var->bits_per_pixel) != 0)
	{
		return -EINVAL;
	}

	if (var->xres_virtual < var->xoffset + var->xres)
		var->xres_virtual = var->xoffset + var->xres;
	if (var->yres_virtual < var->yoffset + var->yres)
		var->yres_virtual = var->yoffset + var->yres;

	/*
	 *  Memory limit
	 */
	line_length = get_line_length(var->xres_virtual, var->bits_per_pixel);
	if (line_length * var->yres_virtual > info->fix.smem_len)
	{
		return -ENOMEM;
	}

	/*
	 * Now that we checked it we alter var. The reason being is that the video
	 * mode passed in might not work but slight changes to it might make it 
	 * work. This way we let the user know what is acceptable.
	 */
	switch (var->bits_per_pixel) 
	{
		case 8:
			var->red.offset = 0;
			var->red.length = 8;
			var->green.offset = 0;
			var->green.length = 8;
			var->blue.offset = 0;
			var->blue.length = 8;
			var->transp.offset = 0;
			var->transp.length = 0;
			break;

		case 16: 
			if (var->transp.length == 4) {    /* ARGB4444 */
				var->red.offset = 8;
				var->red.length = 4;
				var->green.offset = 4;
				var->green.length = 4;
				var->blue.offset = 0;
				var->blue.length = 4;
				var->transp.offset = 12;
				var->transp.length = 4;
			} else if(var->transp.length) {   /* ARGB1555 */
				var->red.offset = 10;
				var->red.length = 5;
				var->green.offset = 5;
				var->green.length = 5;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 15;
				var->transp.length = 1;
			} else {        		  /* RGB 565 */
				var->red.offset = 11;
				var->red.length = 5;
				var->green.offset = 5;
				var->green.length = 6;
				var->blue.offset = 0;
				var->blue.length = 5;
				var->transp.offset = 0;
				var->transp.length = 0;
			}
			break;
			
#if defined(CONFIG_ARCH_ORION_CSM1201)
		case 32:
			/*var->red.offset = 0;
			var->red.length = 8;
			var->green.offset = 8;
			var->green.length = 8;
			var->blue.offset = 16;
			var->blue.length = 8;
			var->transp.offset = 24;
			var->transp.length = 8;*/
			var->red.offset = 16;
			var->red.length = 8;
			var->green.offset = 8;
			var->green.length = 8;
			var->blue.offset = 0;
			var->blue.length = 8;
			var->transp.offset = 24;
			var->transp.length = 8;
#endif
			break;
	}

	var->red.msb_right = 0;
	var->green.msb_right = 0;
	var->blue.msb_right = 0;
	var->transp.msb_right = 0;

	return 0;
}

/* This routine actually sets the video mode. It's in here where we
 * the hardware state info->par and fix which can be affected by the 
 * change in par. For this driver it doesn't do much. 
 */
static int orionfb_set_par(struct fb_info *info)
{
	info->fix.visual      = (info->var.bits_per_pixel <= 8) ? FB_VISUAL_PSEUDOCOLOR : FB_VISUAL_TRUECOLOR;
	info->fix.line_length = get_line_length(info->var.xres_virtual,
			info->var.bits_per_pixel);

#if defined(CONFIG_ARCH_ORION_CSM1201)
	if(info->node >1){
		orion->fbs_lst[info->node -2].info->fix.line_length = orion->fbs_lst[info->node -2].info->var.xres *
									(info->var.bits_per_pixel / 8);

		orion->fbs_lst[info->node -2].info->var.bits_per_pixel = info->var.bits_per_pixel;
		orion->fbs_lst[info->node -2].info->var.transp.offset = info->var.transp.offset;
		orion->fbs_lst[info->node -2].info->var.transp.length = info->var.transp.length;
		orion->fbs_lst[info->node -2].info->var.transp.msb_right = info->var.transp.msb_right;
		orion->fbs_lst[info->node -2].info->var.red.offset = info->var.red.offset;
		orion->fbs_lst[info->node -2].info->var.red.length = info->var.red.length;
		orion->fbs_lst[info->node -2].info->var.red.msb_right = info->var.red.msb_right;
		orion->fbs_lst[info->node -2].info->var.green.offset = info->var.green.offset;
		orion->fbs_lst[info->node -2].info->var.green.length = info->var.green.length;
		orion->fbs_lst[info->node -2].info->var.green.msb_right = info->var.green.msb_right;
		orion->fbs_lst[info->node -2].info->var.blue.offset = info->var.blue.offset;
		orion->fbs_lst[info->node -2].info->var.blue.length = info->var.blue.length;
		orion->fbs_lst[info->node -2].info->var.blue.msb_right = info->var.blue.msb_right;
	}
#endif
	return orionfb_hw_set_par(info);
}

/*
 * Set a single color register. The values supplied are already
 * rounded down to the hardware's capabilities (according to the
 * entries in the var structure). Return != 0 for invalid regno.
 */
static int orionfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
		u_int transp, struct fb_info *info)
{
	if (regno >= 256) return 1; /* no. of hw registers */

	/*
	 * Program hardware... do anything you want with transp
	 */

	/* grayscale works only partially under directcolor */
	if (info->var.grayscale) {
		/* grayscale = 0.30*R + 0.59*G + 0.11*B */
		red = green = blue =
			(red * 77 + green * 151 + blue * 28) >> 8;
	}

	return orionfb_hw_setcolreg(regno, red, green, blue, transp, info);
}

/*
 * Pan or Wrap the Display
 *
 * This call looks only at xoffset, yoffset and the FB_VMODE_YWRAP flag
 */
static int orionfb_pan_display(struct fb_var_screeninfo *var,
		struct fb_info *info)
{
	if (var->vmode & FB_VMODE_YWRAP) {
		return -EINVAL;     
	} else {
		if (var->xoffset + var->xres > info->var.xres_virtual ||
				var->yoffset + var->yres > info->var.yres_virtual)
			return -EINVAL;
	}

	info->var.xoffset = var->xoffset;
	info->var.yoffset = var->yoffset;

	return orionfb_hw_pan_display(info);
}

/*
 * Blank (disable) the Graphics Layer
 *
 */
static int orionfb_blank(int blank, struct fb_info *info)
{
	return orionfb_hw_blank(blank);
}

static int USE2D_COMP_NOTSCALOR = 1; /* 2D scalor or comp while 576i ? 0:scalor, 1:comp*/
static int orionfb_ioctl(struct inode *inode, struct file *file, u_int cmd,
		u_long arg, struct fb_info *info)
{
	switch(cmd) {
		case FBIO_WAITFORVSYNC:
			return orionfb_hw_waitforvsync(info);

		case FBIO_GFX_ON:
			return orionfb_hw_gfx_on(info, arg);

		case FBIO_GFX_ALPHA:
			return orionfb_hw_gfx_alpha(info, arg);

#if defined(CONFIG_ARCH_ORION_CSM1200)
		case FBIO_GFX_BGCOLOR:
			return orionfb_hw_setbgcolor(arg);
#endif

		case FBIO_Z_ORDER:
			return orionfb_hw_z_order(info, arg); //FIXME@zhongkai's ugly codes

		case FBIO_GFX_COLORKEY_ON:
			return orionfb_hw_colorkey_on(info, arg);

		case FBIO_GFX_COLORKEY_VAL:
			{
				gfx_colorkey col_key = {
					0x00, 0x00, 0x00,
					0x00, 0x00, 0x00
				};

				if (NULL != (void *)arg) 		
					if (copy_from_user(&col_key, (void*)arg, sizeof(col_key))) return -EFAULT;

				return orionfb_hw_colorkey_val(info, &col_key);
			}

		case FBIO_GFX_FLIP:
			{
				gfx2d_scalor_params blit_conf, blit_conf2;

				if (info->node < 2) return -EINVAL; /* FIXME@zhongkai's ugly code. */

				blit_conf.src_rect.left = 0;
				blit_conf.src_rect.top = 0;
				blit_conf.src_rect.right = info->var.xres;
				blit_conf.src_rect.bottom = info->var.yres;
				blit_conf.src_width = info->var.xres;
				blit_conf.src_height = info->var.yres;
				blit_conf.src_color_format = (info->var.bits_per_pixel == 32) ? 6 /* GFX_FMT_ARGB8888 */
					: (info->var.bits_per_pixel == 8) ? 5 /* GFX_FMT_CLUT8 */
					: (info->var.transp.length == 0)  ? 0 /* GFX_FMT_RGB565 */ 
					: (info->var.transp.length == 1)  ? 3 /* GFX_FMT_ARGB1555 */
					: (info->var.transp.length == 4)  ? 1 /* GFX_FMT_ARGB4444 */
					: 4; /* GFX_FMT_CLUT4 */
				blit_conf.src_pitch_line = info->fix.line_length;
				blit_conf.src_bits_pixel = info->var.bits_per_pixel;
				blit_conf.src_phy_addr = info->fix.smem_start;

				blit_conf.dst_rect.left = 0;
				blit_conf.dst_rect.top = 0;
				blit_conf.dst_rect.right = orion->fbs_lst[info->node-2].info->var.xres;
				blit_conf.dst_rect.bottom = orion->fbs_lst[info->node-2].info->var.yres;
				blit_conf.dst_width = orion->fbs_lst[info->node-2].info->var.xres;
				blit_conf.dst_height = orion->fbs_lst[info->node-2].info->var.yres;
				blit_conf.dst_color_format = blit_conf.src_color_format;
				blit_conf.dst_pitch_line = orion->fbs_lst[info->node-2].info->fix.line_length;
				blit_conf.dst_bits_pixel = orion->fbs_lst[info->node-2].info->var.bits_per_pixel;
				blit_conf.dst_phy_addr = orion->fbs_lst[info->node-2].info->fix.smem_start;

				if (NULL != (void *)arg) {	
					if (copy_from_user(&blit_conf2, (void*)arg, sizeof(blit_conf))) 
						return -EFAULT;

					blit_conf.src_rect.left = blit_conf2.src_rect.left;
					blit_conf.src_rect.top = blit_conf2.src_rect.top;
					blit_conf.src_rect.right = blit_conf2.src_rect.right;
					blit_conf.src_rect.bottom = blit_conf2.src_rect.bottom;

					blit_conf.dst_rect.left = blit_conf2.dst_rect.left;
					blit_conf.dst_rect.top = blit_conf2.dst_rect.top;
					blit_conf.dst_rect.right = blit_conf2.dst_rect.right;
					blit_conf.dst_rect.bottom = blit_conf2.dst_rect.bottom;

					goto do_scaler; 
				}

				/* for 576&480 */
				if (((blit_conf.src_rect.top-blit_conf.src_rect.bottom) !=
				     (blit_conf.dst_rect.top-blit_conf.dst_rect.bottom)) || 
				    ((blit_conf.src_rect.right-blit_conf.src_rect.left) != 
				     (blit_conf.dst_rect.right-blit_conf.dst_rect.left)))

					goto do_scaler;

				if ((orion->fbs_lst[info->node-2].info->var.yres) <= 576 &&
				    (1 == orion->fbs_lst[info->node-2].info->var.vmode)) {

					if(USE2D_COMP_NOTSCALOR == 0) { /* force 2D scalor */
						blit_conf.src_rect.left = 1;
						blit_conf.src_rect.top = 1;
						blit_conf.src_rect.right = info->var.xres - 1;
						blit_conf.src_rect.bottom = info->var.yres - 1;

					} else if(USE2D_COMP_NOTSCALOR == 1) { /* 2D comp */
						gfx2d_comp_params comp_param;
						comp_param.src1_rect.left = comp_param.src0_rect.left = blit_conf.src_rect.left;
						comp_param.src1_rect.top = comp_param.src0_rect.top = blit_conf.src_rect.top;
						comp_param.src1_rect.right = comp_param.src0_rect.right = blit_conf.src_rect.right;
						comp_param.src1_rect.bottom = comp_param.src0_rect.bottom = blit_conf.src_rect.bottom;
						comp_param.src1_width = comp_param.src0_width = 
							blit_conf.src_rect.right - blit_conf.src_rect.left;// info->var.xres;
						comp_param.src1_height = comp_param.src0_height = 
							blit_conf.src_rect.bottom - blit_conf.src_rect.top;//info->var.yres;
						comp_param.src1_color_format = comp_param.src0_color_format = 
									       (info->var.bits_per_pixel == 32)  ? 6 /* GFX_FMT_ARGB8888 */
									       : (info->var.bits_per_pixel == 8) ? 5 /* GFX_FMT_CLUT8 */
									       : (info->var.transp.length == 0)  ? 0 /* GFX_FMT_RGB565 */ 
									       : (info->var.transp.length == 1)  ? 3 /* GFX_FMT_ARGB1555 */
									       : (info->var.transp.length == 4)  ? 1 /* GFX_FMT_ARGB4444 */
									       : 4; /* GFX_FMT_CLUT4 */
						comp_param.src1_pitch_line = comp_param.src0_pitch_line = info->fix.line_length;
						comp_param.src1_bits_pixel = comp_param.src0_bits_pixel = info->var.bits_per_pixel;
						comp_param.src0_phy_addr = info->fix.smem_start;
						comp_param.src1_phy_addr = info->fix.smem_start + comp_param.src1_pitch_line;

						comp_param.dst_rect.left = blit_conf.dst_rect.left;
						comp_param.dst_rect.top = blit_conf.dst_rect.top;
						comp_param.dst_rect.right = blit_conf.dst_rect.right;
						comp_param.dst_rect.bottom = blit_conf.dst_rect.bottom;
						comp_param.dst_width = 
							blit_conf.dst_rect.right - blit_conf.dst_rect.left;// orion->fbs_lst[info->node-2].info->var.xres;
						comp_param.dst_height = 
							blit_conf.dst_rect.bottom - blit_conf.dst_rect.top;// orion->fbs_lst[info->node-2].info->var.yres;
						comp_param.dst_color_format = comp_param.src0_color_format;
						comp_param.dst_pitch_line = orion->fbs_lst[info->node-2].info->fix.line_length;
						comp_param.dst_bits_pixel = orion->fbs_lst[info->node-2].info->var.bits_per_pixel;
						comp_param.dst_phy_addr = orion->fbs_lst[info->node-2].info->fix.smem_start;

						DFB_2DComposite(&comp_param);

						return 0;
					}
				}
do_scaler:
				DFB_2DScalor(&blit_conf);
				break;
			}
		default:
			return -EINVAL;
	}

	return 0;
}

/*
 * Initialisation
 */
static void orionfb_platform_release(struct device *device)
{
	/* This is called when the reference count goes to zero. */
}

struct fb_info *gfx, *gfx2, *gfx3, *gfx4;
static struct proc_dir_entry *fb_proc_entry = NULL;

static int fb_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	u32 addr;
	u32 val;

	const char *cmd_line = buffer;;

	if (strncmp("rl", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = df_readl(addr);
		printk(" readw [0x%04x] = 0x%08x \n", addr, val);
	}
	else if (strncmp("wl", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = simple_strtol(&cmd_line[7], NULL, 16);
		df_writel(addr, val);
	}
	else if (strncmp("2dst", cmd_line, 4) == 0) {
		int i;
		for (i = 0; i <= 0x8c; i+=4) {
			val = Gfx_ReadRegs(i/4);
			printk(" 2d status [0x%04x] = 0x%08x \n", i, val);
		}
	}
	else if (strncmp("0", cmd_line, 1) == 0) {
		USE2D_COMP_NOTSCALOR = 0;
	}
	else if (strncmp("1", cmd_line, 1) == 0) {
		USE2D_COMP_NOTSCALOR = 1;
	}

	return count;
}

static int __init orionfb_probe(struct device *device)
{
	int retval = -ENOMEM;

	struct platform_device *dev = to_platform_device(device);

	if (NULL == (gfx = framebuffer_alloc(sizeof(u32) * 256, &dev->dev))) goto GFX_ERR;
	if (NULL == (gfx->screen_base = ioremap(orionfb_fix.smem_start, orionfb_fix.smem_len))) goto MMAP_ERR;

	fb_proc_entry = create_proc_entry("fb_io", 0, NULL);
	if (NULL != fb_proc_entry) {
		fb_proc_entry->write_proc = &fb_proc_write;
	}

	gfx->fbops = &orionfb_ops;
	gfx->var = orionfb_default;
	gfx->fix = orionfb_fix;
	gfx->pseudo_palette = gfx->par;
	gfx->par = NULL;
	gfx->flags = FBINFO_FLAG_DEFAULT;

	if (fb_alloc_cmap(&gfx->cmap, 256, 0) < 0) goto CMAP_ERR;
	if (register_framebuffer(gfx) < 0) goto RF_ERR;

	orion->fbs_lst[0].info = gfx;
	//memset(gfx->screen_base, 0, FB0_SIZE);

	printk(KERN_INFO "fb%d: ORION frame buffer @[0x%lx, 0x%lx] size 0x%lx\n",
			gfx->node, gfx->fix.smem_start, (u_long)gfx->screen_base, (u_long)gfx->fix.smem_len);

	/* to register the second framebuffer */
	if (NULL == (gfx2 = framebuffer_alloc(sizeof(u32) * 256, &dev->dev))) goto GFX2_ERR;
	if (NULL == (gfx2->screen_base = ioremap(orionfb_fix2.smem_start, orionfb_fix2.smem_len))) goto MMAP2_ERR;

	gfx2->fbops = &orionfb_ops;
	gfx2->var = orionfb_default;
	gfx2->fix = orionfb_fix2;
	gfx2->pseudo_palette = gfx2->par;
	gfx2->par = NULL;
	gfx2->flags = FBINFO_FLAG_DEFAULT;

	if (fb_alloc_cmap(&gfx2->cmap, 256, 0) < 0) goto CMAP2_ERR;
	if (register_framebuffer(gfx2) < 0) goto RF2_ERR;

	orion->fbs_lst[1].info = gfx2;
	memset(gfx2->screen_base, 0, FB1_SIZE);

 	Gfx_2DInit();

	printk(KERN_INFO "fb%d: ORION frame buffer @[0x%lx, 0x%lx] size 0x%lx\n",
			gfx2->node, gfx2->fix.smem_start, (u_long)gfx2->screen_base, (u_long)gfx2->fix.smem_len);

	/* to register the third framebuffer */
	if (0 == orionfb_fix3.smem_len) orionfb_fix3.smem_len = 0x1; /* SunHe: <BUG fix>, when set to zero, kernel oops. */
	if (NULL == (gfx3 = framebuffer_alloc(sizeof(u32) * 256, &dev->dev))) goto GFX3_ERR;
	if (NULL == (gfx3->screen_base = ioremap(orionfb_fix3.smem_start, orionfb_fix3.smem_len))) goto MMAP3_ERR;

	gfx3->fbops = &orionfb_ops;
	gfx3->var = orionfb_default;
	gfx3->fix = orionfb_fix3;
	gfx3->pseudo_palette = gfx3->par;
	gfx3->par = NULL;
	gfx3->flags = FBINFO_FLAG_DEFAULT;

	if (fb_alloc_cmap(&gfx3->cmap, 256, 0) < 0) goto CMAP3_ERR;
	if (register_framebuffer(gfx3) < 0) goto RF3_ERR;

	orion->fbs_lst[2].info = gfx3;
	memset(gfx3->screen_base, 0, orionfb_fix3.smem_len);

	/* to register the 4th framebuffer */
	if (0 == orionfb_fix4.smem_len) orionfb_fix4.smem_len = 0x1; /* SunHe: <BUG fix>, when set to zero, kernel oops. */
	if (NULL == (gfx4 = framebuffer_alloc(sizeof(u32) * 256, &dev->dev))) goto GFX4_ERR;
	if (NULL == (gfx4->screen_base = ioremap(orionfb_fix4.smem_start, orionfb_fix4.smem_len))) goto MMAP4_ERR;

	gfx4->fbops = &orionfb_ops;
	gfx4->var = orionfb_default;
	gfx4->fix = orionfb_fix4;
	gfx4->pseudo_palette = gfx4->par;
	gfx4->par = NULL;
	gfx4->flags = FBINFO_FLAG_DEFAULT;

	if (fb_alloc_cmap(&gfx4->cmap, 256, 0) < 0) goto CMAP4_ERR;
	if (register_framebuffer(gfx4) < 0) goto RF4_ERR;

	orion->fbs_lst[3].info = gfx4;
	memset(gfx4->screen_base, 0, orionfb_fix4.smem_len);

	/* to initialize the first framebuffer */
#if defined(CONFIG_ARCH_ORION_CSM1201)
	orionfb_hw_init(gfx, 0);
#elif defined(CONFIG_ARCH_ORION_CSM1200)
	orionfb_hw_init(gfx, 1);
#endif
	orionfb_hw_init(gfx2, 0);
	
	return 0;

RF4_ERR:
CMAP4_ERR:
MMAP4_ERR:
GFX4_ERR:
	/* do something here. */
RF3_ERR:
CMAP3_ERR:
MMAP3_ERR:
GFX3_ERR:
	/* do something here. */

RF2_ERR:
	fb_dealloc_cmap(&gfx2->cmap);
CMAP2_ERR:
	iounmap(gfx2->screen_base);
MMAP2_ERR:
GFX2_ERR:
	framebuffer_release(gfx);
RF_ERR:
	fb_dealloc_cmap(&gfx->cmap);
CMAP_ERR:
	iounmap(gfx->screen_base);
MMAP_ERR:
GFX_ERR:

	return retval;
}

static int orionfb_remove(struct device *device)
{
	int i;
	struct fb_info *info;

	for (i = 0; i < FB_NUMS; i++) {
		info = orion->fbs_lst[i].info;
		if (info) {
			unregister_framebuffer(info);
			iounmap(info->screen_base);
			framebuffer_release(info);
		}
	}

	return 0;
}

static struct device_driver orionfb_driver = {
	.name   = "orionfb",
	.bus    = &platform_bus_type,
	.probe  = orionfb_probe,
	.remove = orionfb_remove,
};

static struct platform_device orionfb_device = {
	.name   = "orionfb",
	.id     = 0,
	.dev    = {
		.release = orionfb_platform_release,
	}
};

static int __init orionfb_init(void)
{
	int ret = 0;

	ret = driver_register(&orionfb_driver);

	if (!ret) {
		ret = platform_device_register(&orionfb_device);
		if (ret)
			driver_unregister(&orionfb_driver);
	}

	return ret;
}

module_init(orionfb_init);

#ifdef MODULE
static void __exit orionfb_exit(void)
{
	orionfb_hw_exit();

	platform_device_unregister(&orionfb_device);
	driver_unregister(&orionfb_driver);
}

module_exit(orionfb_exit);

MODULE_LICENSE("GPL");

#endif /* MODULE */

