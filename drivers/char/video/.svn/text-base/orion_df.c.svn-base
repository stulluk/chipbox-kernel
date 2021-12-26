#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <asm/semaphore.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

MODULE_AUTHOR("Zhongkai Du, <zhongkai.du@celestialsemi.com>");
MODULE_DESCRIPTION("Celestial Semiconductor Display feeder sub-system driver");
MODULE_LICENSE("GPL");

/* 
 * Register Map2 for video display 
 */
#define DF_UPDATE_REG           51

#define DF_USR_CTRL             71
#define DF_USR_X_TOTAL      	72
#define DF_USR_Y_TOTAL      	73
#define DF_USR_ACTIVE_X     	74
#define DF_USR_ACTIVE_Y     	75
#define DF_USR_SAV_START    	76
#define DF_USR_V0_START_TOP 	77
#define DF_USR_V0_END_TOP   	78
#define DF_USR_V0_START_BOT 	79
#define DF_USR_V0_END_BOT   	80
#define DF_USR_F0_START     	81
#define DF_USR_F0_END       	82
#define DF_USR_HS_START     	83
#define DF_USR_HS_END       	84
#define DF_USR_VS_END_TOP   	85
#define DF_USR_VS_END_BOT   	86
#define DF_USR_IS_HD            87
#define DF_USR_IMG_WIDTH       	88
#define DF_USR_IMG_HEIGHT      	89
#define DF_VIDEO2_X_START      	90    
#define DF_VIDEO2_X_END        	91    
#define DF_VIDEO2_Y_START      	92    
#define DF_VIDEO2_Y_END        	93    
#define DF_VIDEO2_FORMAT       	94    
#define DF_VIDEO2_EN          	95    
#define DF_VIDEO2_DEFAULT_ALPHA	96    

#define DF_CLIP_Y_LOW          	98    
#define DF_CLIP_Y_RANGE        	99    
#define DF_CLIP_C_RANGE        	100   
#define DF_VERTICAL_SEL        	101
#define DF_USR_CTRL_V2         	102
#define DF_VIDEO_SRC_X_CROP    	103
#define DF_VIDEO_SRC_Y_CROP    	104
#define DF_VIDEO2_SRC_X_CROP   	105
#define DF_VIDEO2_SRC_Y_CROP   	106

#define DF_VIDEO_EN             45
#define DF_VIDEO_DEFAULT_ALPHA  46

#define DF_VIDEO_X_START        16
#define DF_VIDEO_X_END          17
#define DF_VIDEO_Y_START        18
#define DF_VIDEO_Y_END          19
#define DF_DISP_ENABLE          20
#define DF_SYNC_MODE            21
#define DF_VIDEO_FORMAT         22

#define DF_GFX1_EN	        23
#define DF_GFX2_EN         	34

#define DF_GFX1_Z_ORDER         26
#define DF_GFX2_Z_ORDER         37
#define DF_VIDEO_Z_ORDER        47
#define DF_BG_Y        		48
#define DF_BG_U        		49
#define DF_BG_V        		50
#define DF_VIDEO2_Z_ORDER      	97    

#define ORION_DISP_BASE		0x41800000
#define ORION_DISP_SIZE		0x00001000
/* 
 * End of Register Map2
 */

#define _VID_EN			0
#define _VID_ALPHA		1
#define _USR_CTRL		2
#define _VID_SRC_X_CROP		3
#define _VID_SRC_Y_CROP		4
#define _VID_X_START		5
#define _VID_X_END		6
#define _VID_Y_START		7
#define _VID_Y_END		8
#define _VID_FORMAT		9
#define _USR_X_TOTAL		10
#define _USR_Y_TOTAL		11
#define _USR_ACTIVE_X		12
#define _USR_ACTIVE_Y		13
#define _USR_SAV_START		14
#define _USR_V_START_TOP	15
#define _USR_V_END_TOP		16
#define _USR_V_START_BOT	17
#define _USR_V_END_BOT		18
#define _USR_F_START		19
#define _USR_F_END		20
#define _USR_HS_START		21
#define _USR_HS_END		22
#define _USR_VS_END_TOP		23
#define _USR_VS_END_BOT		24
#define _USR_IS_HD		25
#define _USR_IMG_WIDTH		26
#define _USR_IMG_HEIGHT		27

static int vid_regs_lst[2][28] = {
	{
		DF_VIDEO_EN,		DF_VIDEO_DEFAULT_ALPHA,	DF_USR_CTRL,	  DF_VIDEO_SRC_X_CROP,
		DF_VIDEO_SRC_Y_CROP,	DF_VIDEO_X_START,	DF_VIDEO_X_END,	  DF_VIDEO_Y_START,
		DF_VIDEO_Y_END,		DF_VIDEO_FORMAT,	DF_USR_X_TOTAL,	  DF_USR_Y_TOTAL,
		DF_USR_ACTIVE_X,	DF_USR_ACTIVE_Y,	DF_USR_SAV_START, DF_USR_V0_START_TOP,
		DF_USR_V0_END_TOP,	DF_USR_V0_START_BOT,	DF_USR_V0_END_BOT,DF_USR_F0_START,
		DF_USR_F0_END,		DF_USR_HS_START,	DF_USR_HS_END,	  DF_USR_VS_END_TOP,
		DF_USR_VS_END_BOT,	DF_USR_IS_HD,		DF_USR_IMG_WIDTH, DF_USR_IMG_HEIGHT
	},
	{
		DF_VIDEO2_EN,		DF_VIDEO2_DEFAULT_ALPHA,DF_USR_CTRL_V2,	  DF_VIDEO2_SRC_X_CROP,
		DF_VIDEO2_SRC_Y_CROP,	DF_VIDEO2_X_START,	DF_VIDEO2_X_END,  DF_VIDEO2_Y_START,
		DF_VIDEO2_Y_END,	DF_VIDEO2_FORMAT,	DF_USR_X_TOTAL,	  DF_USR_Y_TOTAL,
		DF_USR_ACTIVE_X,	DF_USR_ACTIVE_Y,	DF_USR_SAV_START, DF_USR_V0_START_TOP,
		DF_USR_V0_END_TOP,	DF_USR_V0_START_BOT,	DF_USR_V0_END_BOT,DF_USR_F0_START,
		DF_USR_F0_END,		DF_USR_HS_START,	DF_USR_HS_END,	  DF_USR_VS_END_TOP,
		DF_USR_VS_END_BOT,	DF_USR_IS_HD,		DF_USR_IMG_WIDTH, DF_USR_IMG_HEIGHT
	}
};

/* 
 * Z-Order numbers are from hardware, don't redefine them! 
 */
#define Z_ORDER_V0_V1_G         	0x0123
#define Z_ORDER_V0_G_V1         	0x0213
#define Z_ORDER_V1_V0_G         	0x1023
#define Z_ORDER_V1_G_V0         	0x2013
#define Z_ORDER_G_V0_V1         	0x1203
#define Z_ORDER_G_V1_V0         	0x2103

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

static volatile unsigned long  *disp_base = NULL;
DEFINE_SPINLOCK(orion_df_lock);

/* 
 * the implementations of display functions
 */
static int __df_disp_init(void)
{
	disp_base = (unsigned long *)ioremap(ORION_DISP_BASE, ORION_DISP_SIZE);
	if (NULL == disp_base) 
		return -1;;

	return 0;
}

static void __df_disp_deinit(void)
{
	iounmap((void *)disp_base);
}

int __df_disp_enable(int en_flags)
{
 	disp_base[DF_UPDATE_REG] = 0x1;
	disp_base[DF_DISP_ENABLE] = en_flags ? 0x1 : 0x0;
	disp_base[DF_UPDATE_REG] = 0x0;

	return 0;
}

static int __df_vid_on(int layer, unsigned int on_flags) 
{
 	disp_base[DF_UPDATE_REG] = 0x1;
	disp_base[vid_regs_lst[layer][_VID_EN]] = on_flags ? 1 : 0;
	disp_base[DF_UPDATE_REG] = 0x0;
	
	return 0;  
}

static int __df_gfx_on(int layer, unsigned int on_flags) 
{
 	disp_base[DF_UPDATE_REG] = 0x1;
	if(layer == 0)
	    disp_base[DF_GFX1_EN] = on_flags ? 1 : 0;
	else if(layer == 1)
	    disp_base[DF_GFX2_EN] = on_flags ? 1 : 0;
	disp_base[DF_UPDATE_REG] = 0x0;
	
	return 0;  
}

static int __df_disp_alpha(int layer, unsigned int alpha) 
{
 	disp_base[DF_UPDATE_REG] = 0x1;
	disp_base[vid_regs_lst[layer][_VID_ALPHA]] = alpha & 0xff;
	disp_base[DF_UPDATE_REG] = 0x0;
	
	return 0;  
}

static int __df_disp_src_fmt(int layer, unsigned int fmt) 
{
 	disp_base[DF_UPDATE_REG] = 0x1;
    	disp_base[vid_regs_lst[layer][_USR_CTRL]] = fmt ? 
						    disp_base[vid_regs_lst[layer][_USR_CTRL]] | (0x1 << 6): /* H.264 */
                                       		    disp_base[vid_regs_lst[layer][_USR_CTRL]] & ~(0x1 << 6);
	disp_base[DF_UPDATE_REG] = 0x0;
	
	return 0;  
}

struct df_rect
{
        int left;
        int right;
        int top;
        int bottom;
};

struct df_output_pos {
	struct df_rect src;
	struct df_rect dst;
};

static int __df_disp_position(int layer, struct df_output_pos *pos)
{
    disp_base[DF_UPDATE_REG] = 0x1;

    disp_base[vid_regs_lst[layer][_VID_SRC_X_CROP]] = (pos->src.right - pos->src.left + 1) << 16 | pos->src.left;
    disp_base[vid_regs_lst[layer][_VID_SRC_Y_CROP]] = (pos->src.bottom - pos->src.top + 1) << 16 | pos->src.top;

    disp_base[vid_regs_lst[layer][_VID_X_START]] = (pos->dst.left & ~0xf); 	/* 16pixels aligned */
    disp_base[vid_regs_lst[layer][_VID_X_END]] = (pos->dst.right + 1) & ~0x1; 	/* 2pixels aligned */
    disp_base[vid_regs_lst[layer][_VID_Y_START]] = (pos->dst.top & ~0xf); 	/* 16pixels aligned */
    disp_base[vid_regs_lst[layer][_VID_Y_END]] = (pos->dst.bottom + 1) & ~0x1; 	/* 2pixels aligned */   

	if ((pos->dst.bottom - pos->dst.top) > 600)        // to fix chroma bug in 720P mode, to use 1 tap filter for vertical
		disp_base[DF_VERTICAL_SEL] = 0x2020;    // 2-tap vertial scalar only at even Y line for more than 600 lines
	else
		disp_base[DF_VERTICAL_SEL] = 0x2222;    // 2-tap vertical scalar for less than 600 lines

    disp_base[DF_UPDATE_REG] = 0x0;

    return 0;
}

/*--------------------------------------------------
* struct df_vid_crop {
* 	unsigned char layer_id;
* 	struct df_rect rect;
* };
* 
* static int __df_disp_crop(int layer, struct df_rect *rect)
* {
*     disp_base[DF_UPDATE_REG] = 0x1;
* 
*     disp_base[vid_regs_lst[layer][_VID_X_START]] = (rect->left & ~0xf); 	/ * 16pixels aligned * /
*     disp_base[vid_regs_lst[layer][_VID_X_END]] = (rect->right + 1) & ~0x1; 	/ * 2pixels aligned * /
*     disp_base[vid_regs_lst[layer][_VID_Y_START]] = (rect->top & ~0xf); 		/ * 16pixels aligned * /
*     disp_base[vid_regs_lst[layer][_VID_Y_END]] = (rect->bottom + 1) & ~0x1; 	/ * 2pixels aligned * /   
* 
*     disp_base[DF_UPDATE_REG] = 0x0;
* 
*     return 0;
* }
*--------------------------------------------------*/

#define VID_FMT_PAL 		3
#define VID_FMT_NTSC 		0
#define VID_FMT_720P 		1
#define VID_FMT_720P_50FPS 	5
#define VID_FMT_1080I 		2
#define VID_FMT_1080I_25FPS 	4
#define VID_FMT_DOMMY       6
#define VID_FMT_USR_DEF 	7
#define VID_FMT_576P 		0x27 	/* Derived From USR_DEF */
#define VID_FMT_480P 		0x17 	/* Derived From USR_DEF */
#define VID_FMT_1080P_24    0x37
#define VID_FMT_1080P_25    0x47
#define VID_FMT_1080P_30    0x57

int video_disp_out_fmt(int layer, unsigned int fmt_t)
{
	unsigned short  usr_x_total;
	unsigned short	usr_y_total;
	unsigned short	usr_x_active;
	unsigned short	usr_y_active;
	unsigned short	usr_sav_start;
	unsigned short	usr_vo_start_top;
	unsigned short	usr_vo_end_top;
	unsigned short	usr_fo_start;
	unsigned short	usr_fo_end;
	unsigned short	usr_hs_start;
	unsigned short	usr_hs_end;
	unsigned short	usr_vs_end_top;

	static unsigned old_fmt = 0xffffffff;

	int fmt;
	int tve_map2_vid[] = {
		VID_FMT_PAL,
		VID_FMT_NTSC,
		VID_FMT_576P,
		VID_FMT_480P,
		VID_FMT_720P_50FPS,
		VID_FMT_720P,
		VID_FMT_1080I_25FPS,
		VID_FMT_1080I,
		VID_FMT_PAL, /* SECAM */
		VID_FMT_NTSC, /* PAL-M */
		VID_FMT_PAL, /* PAL-N */
		VID_FMT_PAL,  /* PAL-CN*/
		VID_FMT_1080P_24,
		VID_FMT_1080P_25,
		VID_FMT_1080P_30
	};

	if (fmt_t >= sizeof(tve_map2_vid)/sizeof(tve_map2_vid[0])) return 0;

	fmt = tve_map2_vid[fmt_t]; /* FIXME@zhongkai's ugly code */

	if (fmt == old_fmt) return 0;

//     __df_disp_enable(0);  /*Added it to avoid possible problem for clock changed, if we change different formats*/
//     msleep(20);
// 
#if 0 // it's not tested, so comment it 
    if ((old_fmt == VID_FMT_PAL || old_fmt ==VID_FMT_NTSC || old_fmt == VID_FMT_1080I || old_fmt == VID_FMT_1080I_25FPS) 
        && (fmt == VID_FMT_720P || fmt == VID_FMT_720P_50FPS || fmt == VID_FMT_480P || fmt == VID_FMT_576P)){
        
        disp_base[DF_UPDATE_REG] = 0x1;
        disp_base[vid_regs_lst[layer][_VID_FORMAT]] = VID_FMT_DOMMY;
        disp_base[DF_UPDATE_REG] = 0x0;
        msleep(20);

    } /* The code is to avoid a df bug from I-P for 656 output*/

#endif 

	switch(fmt) 
	{
		case VID_FMT_PAL:
		case VID_FMT_NTSC:
		case VID_FMT_720P:
		case VID_FMT_720P_50FPS:
		case VID_FMT_1080I:
		case VID_FMT_1080I_25FPS:
			disp_base[DF_UPDATE_REG] = 0x1;
			disp_base[vid_regs_lst[layer][_VID_FORMAT]] = fmt;
			if (fmt == VID_FMT_720P || fmt == VID_FMT_720P_50FPS)
				disp_base[DF_VERTICAL_SEL] = 0x2020;
			else
				disp_base[DF_VERTICAL_SEL] = 0x2222;
			disp_base[DF_UPDATE_REG] = 0x0;
			break;

		case VID_FMT_480P:
		case VID_FMT_576P:
			if (fmt == VID_FMT_480P)
			{
				usr_x_total = 858;
				usr_y_total = 525;
				usr_x_active= 720;
				usr_y_active= 483;
			} else {
				usr_x_total = 864;
				usr_y_total = 625;
				usr_x_active= 720;
				usr_y_active= 576;
			}

			usr_sav_start	= usr_x_total - usr_x_active - 4;
			usr_vo_start_top= 25;
			usr_vo_end_top	= usr_vo_start_top + usr_y_active;
			usr_fo_start	= 0;
			usr_fo_end	= usr_y_total;
			usr_hs_start	= 110;
			usr_hs_end	= usr_hs_start + 40;
			usr_vs_end_top	= 5;

			disp_base[DF_UPDATE_REG] = 0x1;

			disp_base[vid_regs_lst[layer][_VID_FORMAT]] = VID_FMT_USR_DEF;
			disp_base[vid_regs_lst[layer][_USR_X_TOTAL]] = usr_x_total;
			disp_base[vid_regs_lst[layer][_USR_Y_TOTAL]] = usr_y_total;
			disp_base[vid_regs_lst[layer][_USR_ACTIVE_X]] = usr_x_active;
			disp_base[vid_regs_lst[layer][_USR_ACTIVE_Y]] = usr_y_active;
			disp_base[vid_regs_lst[layer][_USR_SAV_START]] = usr_sav_start;
			disp_base[vid_regs_lst[layer][_USR_V_START_TOP]] = usr_vo_start_top;
			disp_base[vid_regs_lst[layer][_USR_V_END_TOP]] = usr_vo_end_top;
			disp_base[vid_regs_lst[layer][_USR_V_START_BOT]] = usr_vo_start_top;
			disp_base[vid_regs_lst[layer][_USR_V_END_BOT]] = usr_vo_end_top;
			disp_base[vid_regs_lst[layer][_USR_F_START]] = usr_fo_start;
			disp_base[vid_regs_lst[layer][_USR_F_END]] = usr_fo_end;
			disp_base[vid_regs_lst[layer][_USR_HS_START]] = usr_hs_start;
			disp_base[vid_regs_lst[layer][_USR_HS_END]] = usr_hs_end;
			disp_base[vid_regs_lst[layer][_USR_VS_END_TOP]] = usr_vs_end_top;
			disp_base[vid_regs_lst[layer][_USR_VS_END_BOT]] = usr_vs_end_top;
			disp_base[vid_regs_lst[layer][_USR_IS_HD]] = 0x1;
			disp_base[vid_regs_lst[layer][_USR_IMG_WIDTH]] = usr_x_active;
			disp_base[vid_regs_lst[layer][_USR_IMG_HEIGHT]] = usr_y_active;
			disp_base[DF_VERTICAL_SEL] = 0x2222;    // 2-tap for vertical scalar filter

			disp_base[DF_UPDATE_REG] = 0x0;

			break;
    case VID_FMT_1080P_24:
    case VID_FMT_1080P_25:
    case VID_FMT_1080P_30:
        usr_x_active= 1920;
        usr_y_active= 1080;
        
        if (fmt == VID_FMT_1080P_24) {
            usr_x_total = 830 + usr_x_active;
            usr_y_total = 45 + usr_y_active;
            usr_hs_start = 638;
            usr_hs_end = usr_hs_start + 44;
            
            printk("debug: vid format is set to 1080p 24Hz\n");
        }
        else if (fmt == VID_FMT_1080P_25) {
            usr_x_total = 720 + usr_x_active;
            usr_y_total = 45 + usr_y_active;
            usr_hs_start = 528;
            usr_hs_end = usr_hs_start + 44;
            printk("debug: vid format is set to 1080p 25Hz\n");
        }
        else {
            usr_x_total = 280 + usr_x_active;
            usr_y_total = 45 + usr_y_active;
            usr_hs_start = 88;
            usr_hs_end = usr_hs_start + 44;
            printk("debug: vid format is set to 1080p 30Hz\n");
        }
        
        
        usr_sav_start = usr_x_total - usr_x_active - 4;
        usr_vo_start_top= 40;
        usr_vo_end_top = usr_vo_start_top + usr_y_active;
        usr_fo_start = 0;
        usr_fo_end = usr_y_total;
        
        usr_vs_end_top = 5;
        
        disp_base[DF_UPDATE_REG] = 0x1;
        
        disp_base[vid_regs_lst[layer][_VID_FORMAT]] = VID_FMT_USR_DEF;
        disp_base[vid_regs_lst[layer][_USR_X_TOTAL]] = usr_x_total;
        disp_base[vid_regs_lst[layer][_USR_Y_TOTAL]] = usr_y_total;
        disp_base[vid_regs_lst[layer][_USR_ACTIVE_X]] = usr_x_active;
        disp_base[vid_regs_lst[layer][_USR_ACTIVE_Y]] = usr_y_active;
        disp_base[vid_regs_lst[layer][_USR_SAV_START]] = usr_sav_start;
        disp_base[vid_regs_lst[layer][_USR_V_START_TOP]] =usr_vo_start_top;
        disp_base[vid_regs_lst[layer][_USR_V_END_TOP]] = usr_vo_end_top;
        disp_base[vid_regs_lst[layer][_USR_V_START_BOT]] =usr_vo_start_top;
        disp_base[vid_regs_lst[layer][_USR_V_END_BOT]] = usr_vo_end_top;
        disp_base[vid_regs_lst[layer][_USR_F_START]] = usr_fo_start;
        disp_base[vid_regs_lst[layer][_USR_F_END]] = usr_fo_end;
        disp_base[vid_regs_lst[layer][_USR_HS_START]] = usr_hs_start;
        disp_base[vid_regs_lst[layer][_USR_HS_END]] = usr_hs_end;
        disp_base[vid_regs_lst[layer][_USR_VS_END_TOP]] =usr_vs_end_top;
        disp_base[vid_regs_lst[layer][_USR_VS_END_BOT]] =usr_vs_end_top;
        disp_base[vid_regs_lst[layer][_USR_IS_HD]] = 0x1;
        disp_base[vid_regs_lst[layer][_USR_IMG_WIDTH]] = usr_x_active;
        disp_base[vid_regs_lst[layer][_USR_IMG_HEIGHT]] = usr_y_active;
	 disp_base[DF_VERTICAL_SEL] = 0x2222;    // 2-tap vertical scalar filter

        disp_base[DF_UPDATE_REG] = 0x0;
        break;
	default:
//             __df_disp_enable(1);
		return -ENXIO;
	}
// 
//     __df_disp_enable(1);
	old_fmt = fmt;

	return 0;
}

int orion_df_zorder(unsigned int arg)
{
    disp_base[DF_UPDATE_REG]      = 0x1;

    disp_base[DF_VIDEO_Z_ORDER]   = (arg >> 12) & 0xf;
    disp_base[DF_VIDEO2_Z_ORDER]  = (arg >>  8) & 0xf;
    disp_base[DF_GFX1_Z_ORDER]    = (arg >>  4) & 0xf;
    disp_base[DF_GFX2_Z_ORDER]    = (arg >>  0) & 0xf;

    disp_base[DF_UPDATE_REG]      = 0x0;

    return 0;
}

static int orion_df_setbgcolor(unsigned int arg)
{
    disp_base[DF_UPDATE_REG]      = 0x1;

    disp_base[DF_BG_Y]   = (arg >> 16) & 0xff;
    disp_base[DF_BG_U]   = (arg >> 8) & 0xff;
    disp_base[DF_BG_V]   = (arg >> 0) & 0xff;

    disp_base[DF_UPDATE_REG]      = 0x0;

    return 0;
}
	
static int orion_df_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	unsigned long flags;
	struct df_output_pos out_pos;
	//struct df_vid_crop vid_crop;

	switch (cmd) {
		case CSDF_IOC_DISP_ON:
			spin_lock_irqsave(&orion_df_lock, flags);
			__df_disp_enable(1);
			spin_unlock_irqrestore(&orion_df_lock, flags);

			break;

		case CSDF_IOC_DISP_OFF:
			spin_lock_irqsave(&orion_df_lock, flags);
			__df_disp_enable(0);
			spin_unlock_irqrestore(&orion_df_lock, flags);

			break;

		case CSDF_IOC_DISP_VID_ON:
			spin_lock_irqsave(&orion_df_lock, flags);
			__df_vid_on(0, 1);
			spin_unlock_irqrestore(&orion_df_lock, flags);

			break;

		case CSDF_IOC_DISP_VID_OFF:
			spin_lock_irqsave(&orion_df_lock, flags);
			__df_vid_on(0, 0);
			spin_unlock_irqrestore(&orion_df_lock, flags);

			break;

		case CSDF_IOC_DISP_GFX_ON:
			spin_lock_irqsave(&orion_df_lock, flags);
			__df_gfx_on(0, 1);
			spin_unlock_irqrestore(&orion_df_lock, flags);

			break;

		case CSDF_IOC_DISP_GFX_OFF:
			spin_lock_irqsave(&orion_df_lock, flags);
			__df_gfx_on(0, 0);
			spin_unlock_irqrestore(&orion_df_lock, flags);

			break;

		case CSDF_IOC_DISP_POS:
                	if (copy_from_user(&out_pos, (void *)arg, sizeof(struct df_output_pos)))
				return -EFAULT;

			spin_lock_irqsave(&orion_df_lock, flags);
			__df_disp_position(0, &out_pos);
			spin_unlock_irqrestore(&orion_df_lock, flags);

			break;
			
		/* this func is included by CSDF_IOC_DISP_POS, so comment here */
		/*
		case CSDF_IOC_DISP_CROP:
                	if (copy_from_user(&vid_crop, (void *)arg, sizeof(struct df_vid_crop)))
				return -EFAULT;

			spin_lock_irqsave(&orion_df_lock, flags);
			__df_disp_crop(vid_crop.layer_id, &vid_crop.rect);
			spin_unlock_irqrestore(&orion_df_lock, flags);

			break;
		*/
			
		case CSDF_IOC_DISP_VID_ALPHA:
			spin_lock_irqsave(&orion_df_lock, flags);
			__df_disp_alpha(0, arg);
			spin_unlock_irqrestore(&orion_df_lock, flags);

			break;

		case CSDF_IOC_DISP_VIDFMT:
			spin_lock_irqsave(&orion_df_lock, flags);
			__df_disp_src_fmt(0, arg);
			spin_unlock_irqrestore(&orion_df_lock, flags);
			break;

		case CSDF_IOC_Z_ORDER:
			spin_lock_irqsave(&orion_df_lock, flags);
			orion_df_zorder(arg);
			spin_unlock_irqrestore(&orion_df_lock, flags);
			break;

		case CSDF_IOC_DISP_SETBGCOLOR:
			spin_lock_irqsave(&orion_df_lock, flags);
			orion_df_setbgcolor(arg);
			spin_unlock_irqrestore(&orion_df_lock, flags);
			break;

		case CSDF_IOC_DISP_SETMODE:
			spin_lock_irqsave(&orion_df_lock, flags);
			video_disp_out_fmt(0, arg);
			spin_unlock_irqrestore(&orion_df_lock, flags);
			break;

		default:
			break;
	}

	return 0;
}

static struct file_operations orion_df_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= orion_df_ioctl,
};

static struct miscdevice orion_df_miscdev = {
	MISC_DYNAMIC_MINOR,
	"orion_df",
	&orion_df_fops
};

static struct proc_dir_entry *df_proc_entry = NULL;

static int df_proc_write(struct file *file, 
                const char *buffer, unsigned long count, void *data)
{
	u32 addr;
	u32 val;

        const char *cmd_line = buffer;;

        if (strncmp("rl", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		printk("Reg val = 0x%x\n", disp_base[addr>>2]);
        }
        else if (strncmp("wl", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = simple_strtol(&cmd_line[7], NULL, 16);
		disp_base[addr>>2] = val;
        }
        else if (strncmp("bg", cmd_line, 2) == 0) {
            printk("Y:0x%02x, U:0x%02x, V:0x%02x\n",(unsigned int)disp_base[DF_BG_Y],(unsigned int)disp_base[DF_BG_U],(unsigned int)disp_base[DF_BG_V]);
	    printk("V1:%s, V2:%s, G1:%s, G2:%s\n",\
		    disp_base[DF_VIDEO_EN] ? "enable" : "disable", 
		    disp_base[DF_VIDEO2_EN] ? "enable" : "disable",
		    disp_base[23] ? "enable" : "disable",
		    disp_base[34] ? "enable" : "disable");
        }

        return count;
}

int __init orion_df_init(void)
{
	int ret = 0;

	ret = -ENODEV;
	if (misc_register(&orion_df_miscdev)) 
		goto ERR_NODEV;

	/* initialize display */
	__df_disp_init();
	__df_disp_enable(1);

	df_proc_entry = create_proc_entry("df_io", 0, NULL);
	if (NULL != df_proc_entry) {
		df_proc_entry->write_proc = &df_proc_write;
	}

	printk(KERN_INFO "%s: Orion Display feeder driver was initialized, at address@[phyical addr = %08x, size = %x] \n", 
		"orion_df", ORION_DISP_BASE, ORION_DISP_SIZE);

	return 0;

ERR_NODEV:
	return ret;
}	

static void __exit orion_df_exit(void)
{
	__df_disp_deinit();

	misc_deregister(&orion_df_miscdev);
}

module_init(orion_df_init);
module_exit(orion_df_exit);

EXPORT_SYMBOL(video_disp_out_fmt);
EXPORT_SYMBOL(__df_disp_enable);
