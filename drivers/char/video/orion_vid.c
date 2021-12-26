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
#include <linux/sched.h>
#include <asm/semaphore.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

#include <linux/mem_define.h>
#include <linux/device.h>
#include <linux/firmware.h>

#include <linux/fb.h>
#if 0
#if defined (CONFIG_ARCH_ORION_CSM1201)
#include "fw1201/video_data_h264.h"
#include "fw1201/video_data_mpeg2.h"
#include "fw1201/video_data_vib.h"
#include "fw1201/video_text_h264.h"
#include "fw1201/video_text_mpeg2.h"
#include "fw1201/video_text_vib.h"
#else
#include "video_data_h264.h"
#include "video_data_mpeg2.h"
#include "video_data_vib.h"
#include "video_text_h264.h"
#include "video_text_mpeg2.h"
#include "video_text_vib.h"
#endif
#endif

MODULE_AUTHOR("Zhongkai Du, <zhongkai.du@celestialsemi.com>");
MODULE_DESCRIPTION("Celestial Semiconductor Video sub-system driver");
MODULE_LICENSE("GPL");

//#define DRV_VIDEO_DEBUG
//#define DRV_VIDEO_INFO
//#define DRV_VIDEO_TESTINFO
//#define DRV_VIDEO_TESTCTL

#define MEM_SWAP32(x)	( (((x) & 0xff) << 24) | (((x) & 0xff00) <<8) |	(((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24) )

/* Register Map */
#define HOST_IF_VID_CMD             	(0x00)
#define HOST_IF_VID_STA             	(0x01)
#define HOST_IF_VID_ENA       		(0x02)
#define HOST_IF_VID_COP_INT        	(0x03)
#define HOST_IF_VID_COP_MASK        	(0x04)
#define HOST_IF_VID_HOST_INT        	(0x05)
#define HOST_IF_VID_HOST_MASK       	(0x06)

#define HOST_IF_VID_CPBDIRU0           	(0x07)
#define HOST_IF_VID_CPBDIRL0           	(0x08)
#define HOST_IF_VID_CPBDIRRP0          	(0x09)
#define HOST_IF_VID_CPBDIRWP0		(0x0a)
#define HOST_IF_VID_HISTC      		(0x0b)
#define HOST_IF_VID_STC        		(0x0c)
#define HOST_IF_VID_CPBDIRU1           	(0x0d)
#define HOST_IF_VID_CPBDIRL1           	(0x0e)
#define HOST_IF_VID_CPBDIRRP1          	(0x0f)
#define HOST_IF_VID_CPBDIRWP1          	(0x10)
#define HOST_IF_VID_HISTC1          	(0x11)
#define HOST_IF_VID_STC1            	(0x12)

#define HOST_IF_VID_DPB0            	(0x13)
#define HOST_IF_VID_DPB1            	(0x14)
#define HOST_IF_VID_UDB             	(0x15)

#define HOST_IF_VID_DF0_CMD         	(0x16)
#define HOST_IF_VID_DF0_ADDR        	(0x17)
#define HOST_IF_VID_DF0_STA0        	(0x18)
#define HOST_IF_VID_DF0_STA1        	(0x19)
#define HOST_IF_VID_DF0_STA2        	(0x1a)
#define HOST_IF_VID_DF1_CMD         	(0x1b)
#define HOST_IF_VID_DF1_ADDR        	(0x1c)
#define HOST_IF_VID_DF1_STA0        	(0x1d)
#define HOST_IF_VID_DF1_STA1        	(0x1e)
#define HOST_IF_VID_DF1_STA2        	(0x1f)

/* 16 mailbox */
#define HOST_IF_VID_MIPS_MAILBOX_0  	(0x20)
#define HOST_IF_VID_MIPS_MAILBOX_1 	        (0x21)
#define HOST_IF_VID_MIPS_MAILBOX_2  	(0x22)
#define HOST_IF_VID_MIPS_MAILBOX_3  	(0x23)
#define HOST_IF_VID_MIPS_MAILBOX_4  	(0x24)
#define HOST_IF_VID_MIPS_MAILBOX_5  	(0x25)
#define HOST_IF_VID_MIPS_MAILBOX_6  	(0x26)
#define HOST_IF_VID_MIPS_MAILBOX_7  	(0x27)
#define HOST_IF_VID_MIPS_MAILBOX_8  	(0x28)
#define HOST_IF_VID_MIPS_MAILBOX_9  	(0x29)
#define HOST_IF_VID_MIPS_MAILBOX_10 	(0x2a)
#define HOST_IF_VID_MIPS_MAILBOX_11 	(0x2b)
#define HOST_IF_VID_MIPS_MAILBOX_12 	(0x2c)
#define HOST_IF_VID_MIPS_MAILBOX_13 	(0x2d)
#define HOST_IF_VID_MIPS_MAILBOX_14 	(0x2e)
#define HOST_IF_VID_MIPS_MAILBOX_15 	(0x2f)

#define HOST_IF_VID_MIPS_STA0       	(0x30)
#define HOST_IF_VID_MIPS_STA1       	(0x31)
#define HOST_IF_VID_MIPS_STA2       	(0x32)
#define HOST_IF_VID_DEBUG           	(0x33)
#define HOST_IF_VID_EOI             	(0x34)
#define HOST_IF_VID_CMD_AE          	(0x35)
#define HOST_IF_VID_INT_STA         	(0x36)
#define HOST_IF_VID_INT_CLR         	(0x37)
#define HOST_IF_VID_WRITE_STA       	(0x38)
#define HOST_IF_VID_CPB0_TYPE       	(0x39)
#define HOST_IF_VID_CPB1_TYPE       	(0x3a)
#define HOST_IF_VID_TIMER0          	(0x3b)
#define HOST_IF_VID_TIMER1          	(0x3c)

/* for pluto only */
#define HOST_IF_PLUTO_PPBL0_ADDR    	(0x3d)
#define HOST_IF_PLUTO_PPBU0_ADDR    	(0x3e)
#define HOST_IF_PLUTO_PPBRP0_ADDR  	(0x3f)
#define HOST_IF_PLUTO_PPBWP0_ADDR  	(0x40)
#define HOST_IF_PLUTO_PPBL1_ADDR    	(0x41)
#define HOST_IF_PLUTO_PPBU1_ADDR    	(0x42)
#define HOST_IF_PLUTO_PPBRP1_ADDR   	(0x43)
#define HOST_IF_PLUTO_PPBWP1_ADDR   	(0x44)
#define HOST_IF_PLUTO_IPREDLB_ADDR  	(0x45)
#define HOST_IF_PLUTO_MVLB_ADDR     	(0x46)
#define HOST_IF_PLUTO_LFLB_ADDR     	(0x47)
#define HOST_IF_PLUTO_MVBUF0_ADDR   	(0x48)
#define HOST_IF_PLUTO_MVBUF1_ADDR   	(0x49)
#define HOST_IF_PLUTO_FW_BASE_ADDR0 	(0x4a)
#define HOST_IF_PLUTO_FW_END_ADDR0  	(0x4b)
#define HOST_IF_PLUTO_FW_BASE_ADDR1 	(0x4c)
#define HOST_IF_PLUTO_FW_END_ADDR1  	(0x4d)

#define VID_CMD_REG			HOST_IF_VID_MIPS_MAILBOX_2
#define VID_FORMAT_REG			HOST_IF_VID_MIPS_MAILBOX_1
#define VID_MIPS_SAT_REG		HOST_IF_VID_MIPS_STA0
#define VID_VIB_PARA_REG		HOST_IF_VID_MIPS_MAILBOX_9

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
#define DF_USR_IMG_WIDTH      88
#define DF_USR_IMG_HEIGHT     89
#define DF_VIDEO2_X_START     90
#define DF_VIDEO2_X_END        	91
#define DF_VIDEO2_Y_START     92
#define DF_VIDEO2_Y_END        	93
#define DF_VIDEO2_FORMAT      94
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

#define DF_VIDEO_X_START       16
#define DF_VIDEO_X_END           17
#define DF_VIDEO_Y_START       18
#define DF_VIDEO_Y_END           19
#define DF_DISP_ENABLE            20
#define DF_SYNC_MODE              21
#define DF_VIDEO_FORMAT        22

#define ORION_DISP_BASE		0x41800000
#define ORION_DISP_SIZE		0x00001000
/* 
 * End of Register Map2
 */

#define ORION_VIDEO_BASE 	0x41600000
#define ORION_VIDEO_SIZE 	0x100000

#define VID_CMD_START		0
#define VID_CMD_PAUSE		1
#define VID_CMD_STOP		2
#define VID_CMD_INIT		3
#define VID_CMD_FREEZE		4
#define VID_CMD_STEP		5
#define VID_CMD_RESUME		6
#define VID_CMD_SKIP		7
#define VID_CMD_STILLPIC	8
#define VID_CMD_SET_STARTDALAY	9

#define VID_INT_CMDACCEPT       0x1
#define VID_INT_NEWVIDEOFORMAT       0x2
#define VID_INT_NEWASPECTRATIO       0x4
#define VID_INT_STARTSENDCMDTODF       0x8
#define VID_INT_NEWTIMECODE       0x10
#define VID_INT_FINDUSERDATA       0x20
#define VID_INT_ADDFRMTOSWFIFO       0x40
#define VID_INT_FINDDECODERERROR       0x80
#define VID_INT_M2VDRESET       0x200
#define VID_INT_VIBERROR       0x400
#if defined(CONFIG_ARCH_ORION_CSM1201)
#define VID_INT_DF0       0x1000
#define VID_INT_DF1       0x2000
#define VID_INT_SRCFORMAT	0x4000
#endif
#define VID_INT_UNDERFLOW      0x8000

#define VID_CMD_MIN		VID_CMD_START
#define VID_CMD_MAX		VID_CMD_STILLPIC_EXIT

#define CPB_CHANNEL_0		0
#define CPB_CHANNEL_1		1
#define CPB_CHANNEL_NUM		2

#define HOST_ACCESS_VIDEO_REG	0

#if defined(CONFIG_ARCH_ORION_CSM1201)
#define MIPS_ACCESS_VIDEO_REG	7
#else
#define MIPS_ACCESS_VIDEO_REG	1
#endif

#if defined(CONFIG_ARCH_ORION_CSM1201)
#define MIPS_DSRAM_MEM_SIZE	(1024 * 16)	/* 16KB */
#else
#define MIPS_DSRAM_MEM_SIZE	(1024 * 8)	/* 8KB */
#endif
#define MIPS_ISRAM_MEM_SIZE	(1024 * 9)	/* 9KB */

#define INST_OUT_MEM_SIZE	(1024 * 1)
#define INST_MIPS_MEM_SIZE	(1024 * 8)
#define DATA_OUT_MEM_SIZE	(1024 * 8)
#define MIPS_INST_DDR_SIZE_BYTE	(1024 * 128)

#define MIPS_ISRAM_OFFSET	((0x41690000 - ORION_VIDEO_BASE) >> 2)
#define MIPS_DSRAM_OFFSET	((0x416A0000 - ORION_VIDEO_BASE) >> 2)
#define MIPS_RESET_OFFSET	((0x416B0000 - ORION_VIDEO_BASE) >> 2)

#define DDR_CONTROL_MASK_REG	(0x10110020)

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

#if defined(CONFIG_ARCH_ORION_CSM1200)
static int vid_regs_lst[2][28] = {
	{
	 DF_VIDEO_EN, DF_VIDEO_DEFAULT_ALPHA, DF_USR_CTRL, DF_VIDEO_SRC_X_CROP,
	 DF_VIDEO_SRC_Y_CROP, DF_VIDEO_X_START, DF_VIDEO_X_END, DF_VIDEO_Y_START,
	 DF_VIDEO_Y_END, DF_VIDEO_FORMAT, DF_USR_X_TOTAL, DF_USR_Y_TOTAL,
	 DF_USR_ACTIVE_X, DF_USR_ACTIVE_Y, DF_USR_SAV_START, DF_USR_V0_START_TOP,
	 DF_USR_V0_END_TOP, DF_USR_V0_START_BOT, DF_USR_V0_END_BOT, DF_USR_F0_START,
	 DF_USR_F0_END, DF_USR_HS_START, DF_USR_HS_END, DF_USR_VS_END_TOP,
	 DF_USR_VS_END_BOT, DF_USR_IS_HD, DF_USR_IMG_WIDTH, DF_USR_IMG_HEIGHT},
	{
	 DF_VIDEO2_EN, DF_VIDEO2_DEFAULT_ALPHA, DF_USR_CTRL_V2, DF_VIDEO2_SRC_X_CROP,
	 DF_VIDEO2_SRC_Y_CROP, DF_VIDEO2_X_START, DF_VIDEO2_X_END, DF_VIDEO2_Y_START,
	 DF_VIDEO2_Y_END, DF_VIDEO2_FORMAT, DF_USR_X_TOTAL, DF_USR_Y_TOTAL,
	 DF_USR_ACTIVE_X, DF_USR_ACTIVE_Y, DF_USR_SAV_START, DF_USR_V0_START_TOP,
	 DF_USR_V0_END_TOP, DF_USR_V0_START_BOT, DF_USR_V0_END_BOT, DF_USR_F0_START,
	 DF_USR_F0_END, DF_USR_HS_START, DF_USR_HS_END, DF_USR_VS_END_TOP,
	 DF_USR_VS_END_BOT, DF_USR_IS_HD, DF_USR_IMG_WIDTH, DF_USR_IMG_HEIGHT}
};
#endif

struct vid_rect {
	int left;
	int right;
	int top;
	int bottom;
};

struct vid_output_pos {
	struct vid_rect src;
	struct vid_rect dst;
};

#define video_writel(v,a)    	do { *(orion_video_base + (a)) = (v) ;udelay(10); }while(0)
#define video_readl(a)       	*(orion_video_base + (a))

static struct platform_device *video_pdev;
static const struct firmware* video_text_fw_mpeg2=(struct firmware*)NULL;
static const struct firmware* video_text_fw_h264=(struct firmware*)NULL;
static const struct firmware* video_text_fw_vib=(struct firmware*)NULL;
static const struct firmware* video_data_fw_mpeg2=(struct firmware*)NULL;
static const struct firmware* video_data_fw_h264=(struct firmware*)NULL;
static const struct firmware* video_data_fw_vib=(struct firmware*)NULL;

extern void xport_writel(int a,int v);
extern unsigned int xport_readl(int a);
extern unsigned int __audio_readl(unsigned int a);

#if defined(CONFIG_ARCH_ORION_CSM1201)
int __g_videolayer_index = 0;
extern void __df_update_start(void);
extern void __df_update_end(void);
extern unsigned int DF_Read(unsigned int addr);
extern void DF_Write(unsigned int addr, unsigned int data);
extern void DFVideoSetDispWin(int VideoId, struct vid_rect DispRect);
extern int DFVideoIsDispWinChange(int VideoId, struct vid_rect DispRect, struct vid_rect SrcRect);
extern void DFVideoSetSrcCrop(int VideoId, struct vid_rect CropRect);
extern void DFVideoSetLayerAlpha(int VideoId, unsigned char Alpha, unsigned char KeyAlpha0, unsigned char KeyAlpha1);
extern int orion_gpio2_register_module_set(const char * module, unsigned int  orion_module_status);

#define DEFAULT_SCALER
#if defined(DEFAULT_SCALER)
extern int DFVideoSetDefaultCoeff(int VideoId);
#else
extern void DFSetVideoScalerCfg(int VideoId, unsigned int FrameWidth, unsigned int FrameHeight ,unsigned int ScaleFrameWidth, unsigned int ScaleFrameHeight);
#endif
extern void DFVideoEna(int VideoId, int IsEna);
#endif

volatile unsigned long * vib_reset = NULL;
#if defined(CONFIG_ARCH_ORION_CSM1201)
volatile unsigned long * pin_mux0_addr = NULL;
volatile unsigned long * gpio_pin_mux1_addr = NULL;
#endif

#if 1
volatile unsigned long * ddr_port = NULL;
#define ddr_port_disable() do {*ddr_port &= ~(1 << 13);} while(0);
#define ddr_port_enable() do { *ddr_port |= (1 << 13);} while(0);
#else
#define ddr_port_disable() do { \
	volatile unsigned long * ddr_port = (unsigned long*)ioremap(DDR_CONTROL_MASK_REG, 4); \
	*ddr_port &= ~(1 << 13); \
	iounmap((void *)ddr_port); \
	} while(0);

#define ddr_port_enable() do { \
	unsigned long * ddr_port = (unsigned long*)ioremap(DDR_CONTROL_MASK_REG, 4); \
	*ddr_port |= (1 << 13); \
	iounmap((void *)ddr_port); \
	} while(0);
#endif
#define CSVID_IOC_INPUT_MODE 		_IOW('z', 0x01, int)
#define CSVID_IOC_STREAM_TYPE 		_IOW('z', 0x02, int)
#define CSVID_IOC_DECODER_MODE 		_IOW('z', 0x03, int)
#define CSVID_IOC_PLAY_SPEED 		_IOW('z', 0x04, int)
#define CSVID_IOC_FRAME_RATE 		_IOW('z', 0x05, int)
#define CSVID_IOC_PLAY 			_IOW('z', 0x06, int)
#define CSVID_IOC_STOP 			_IOW('z', 0x07, int)
#define CSVID_IOC_PAUSE 		_IOW('z', 0x08, int)
#define CSVID_IOC_FREEZE 		_IOW('z', 0x09, int)
#define CSVID_IOC_STEP 			_IOW('z', 0x0a, int)
#define CSVID_IOC_RESUME 		_IOW('z', 0x0b, int)
#define CSVID_IOC_SKIP 			_IOW('z', 0x0c, int)

#define CSVID_IOC_PTS_ENABLE 		_IOW('z', 0x11, int)
#define CSVID_IOC_PTS_DISABLE 		_IOW('z', 0x12, int)
#define CSVID_IOC_DISP_ON 		_IOW('z', 0x13, int)
#define CSVID_IOC_DISP_OFF 		_IOW('z', 0x14, int)
#define CSVID_IOC_DISP_POS 		_IOW('z', 0x15, struct vid_output_pos)
#define CSVID_IOC_DISP_ALPHA 		_IOW('z', 0x16, int)
#define CSVID_IOC_DISP_BLANK 		_IOW('z', 0x17, int)
#define CSVID_IOC_GET_VIDEOFMT		_IOW('z', 0x18, int)
#define CSVID_IOC_GET_TIMECODE		_IOW('z', 0x19, int)
#define CSVID_IOC_SET_TIMECODE		_IOW('z', 0x1a, int)

#define CSVID_IOC_GET_NOTIFY_TYPE   _IOW('z',0x1c,int)
#define CSVID_IOC_SET_ERROR_LEVEL   _IOW('z',0x1d,int)

#define CSVID_IOC_PMF_RESET     _IOW('z',0x1e,int)
#define CSVID_IOC_PMF_GETCPB_ADDR       _IOW('z',0x1f,int)
#define CSVID_IOC_PMF_GETCPB_SIZE       _IOW('z',0x20,int)
#define CSVID_IOC_PMF_GETDIR_ADDR       _IOW('z',0x21,int)
#define CSVID_IOC_PMF_GETDIR_SIZE       _IOW('z',0x22,int)
#define CSVID_IOC_PMF_SET_DIR_WP        _IOW('z',0x23,int)
#define CSVID_IOC_PMF_GET_DIR_WP        _IOW('z',0x24,int)
#define CSVID_IOC_PMF_SET_DIR_RP         _IOW('z',0x25,int)
#define CSVID_IOC_PMF_GET_DIR_RP         _IOW('z',0x26,int)
#define CSVID_IOC_PMF_SET_CPBDIRU0        _IOW('z',0x27,int)
#define CSVID_IOC_PMF_GET_CPBDIRU0        _IOW('z',0x28,int)
#define CSVID_IOC_PMF_SET_CPBDIRL0         _IOW('z',0x29,int)
#define CSVID_IOC_PMF_GET_CPBDIRL0         _IOW('z',0x2a,int)

#define CSVID_IOC_SET_ASPECTRATIO_STATUS         _IOW('z',0x2b,int)
#define CSVID_IOC_SET_PSCANCROP_STATUS      _IOW('z',0x2c,int)
#define CSVID_IOC_GET_PSCANCROP     _IOW('z',0x2d,int)
#define CSVID_IOC_SET_SYNC_STATUS       _IOW('z',0x2e,int)
#define CSVID_IOC_GET_ASPECTRATIO       _IOW('z',0x2f,int)
#define CSVID_IOC_TRICK_MDOE        _IOW('z',0x30,int)
/* adde by wangxuewewei 2008.03.12*/
/*--------------------------------------------------------*/
#define CSVID_IOC_GET_PTS		_IOW('z', 0x35, long long)
/*--------------------------------------------------------*/
#define CSVID_IOC_SET_FORCE_3TO2_POLLDOW        _IOW('z', 0x36, int)
#define CSVID_IOC_SET_NOWAIT_SYNC        _IOW('z', 0x37, int)
#define CSVID_IOC_SET_STARTDELAY        _IOW('z', 0x38, int)

#define CSVID_IOC_VIB_CONFIG        _IOW('z', 0x39, int)
#define CSVID_IOC_VIB_RESET        _IOW('z', 0x3a, int)
#define CSVID_IOC_SET_USER_DATA_STATUS        _IOW('z', 0x3b, int)
#define CSVID_IOC_GET_USERDATA_ADDR		_IOW('z', 0x3c, int)
#define CSVID_IOC_GET_USERDATA_SIZE	_IOW('z', 0x3d, int)
#define CSVID_IOC_SET_USERDATA_BLOCK_SIZE	_IOW('z', 0x3e, int)

#define CSVID_IOC_SET_ERROR_SKIP_MODE	_IOW('z', 0x3f, int)

#define CSVID_IOC_DISP_POS_BEIYANG		_IOW('z', 0x40, int)
#define CSVID_IOC_DISP_ALPHA_BEIYANG		_IOW('z', 0x41, int)
#if defined(CONFIG_ARCH_ORION_CSM1201)
#define	CSVID_IOC_GET_SRC_VIDEOFMT		_IOW('z', 0x42, int)
#endif

#define CSVID_IOC_SET_UNDERFLOW		_IOW('z', 0x43, int)
#define CSVID_IOC_SET_FORMATCHANGE_REPORT		_IOW('z', 0x44, int)
#define CSVID_IOC_SET_SRCFORMATCHANGE_REPORT		_IOW('z', 0x45, int)

typedef enum {
	STREAM_UNKNOWN = 0,
	STREAM_MPEG2 = 1,
	STREAM_H264 = 2,
	STREAM_VIB
} CPB_STREAM_TYPE;

union video_config_params {
	struct {
		unsigned int pts_enable:1;
		unsigned int trick_mode:1;
		unsigned int decode_mode:2;
		unsigned int channel_num:4;
		unsigned int frame_rate:4;
		unsigned int play_speed:5;

		unsigned int stream_input_mode:1;
		unsigned int error_ignore_mode:1;
		unsigned int error_report_level:3;
              unsigned int sync_flag:1;
              unsigned int force_32_poll_down_flag:1;
		unsigned int ud_stauts:1;
		unsigned int df0:1;
		unsigned int df1:1;
		unsigned int underflow_report:1;
		unsigned int nosyncwait_fast_mode:1;
		unsigned int reserved:2;
	} bits;

	unsigned int val;
};

union video_format_params {
	struct {
		unsigned int pic_width:14;
		unsigned int pic_height:14;
		unsigned int src_frame_rate:4;
	} bits;

	unsigned int val;
};

union video_time_code {
	struct {
		unsigned int pictures:6;
		unsigned int seconds:6;
		unsigned int marker_bit:1;
		unsigned int minutes:6;
		unsigned int hours:5;
		unsigned int drop_frame_flag:1;
		unsigned int reserved:7;
	} bits;

	unsigned int val;
};

union video_flags {
	struct {
		unsigned int stillpicture_flag:1;
		unsigned int timecode_irq:1;
		unsigned int timecode_status:1;
		unsigned int decodeerror_irq:1;
		unsigned int decodeerror_status:1;
		unsigned int aspectratio_irq:1;
		unsigned int aspectratio_status:1;
		unsigned int pscancrop_irq:1;
		unsigned int pscancrop_status:1;
		unsigned int sync_irq:1;
		unsigned int sync_status:1;
		unsigned int sync_irq2:1;
		unsigned int underflow_report_irq:1;
		unsigned int underflow_report_status:1;
		unsigned int formatchange_report_irq:1;
		unsigned int formatchange_report_status:1;
		unsigned int srcformatchange_report_irq:1;
		unsigned int srcformatchange_report_status:1;
		unsigned int reserved:14;
	} bits;

	unsigned int val;
};
static union video_flags vid_flags;

static int gdbinfo_flags = 0;
static union video_config_params vid_configs;

	/* the following values was initialized */
//      .bits = {
//              .pts_enable = 0; /* disable PTS */
//              .trick_mode = 0; /* disable Trick Mode */
//              .decode_mode = 0; /* normal mode (IPB) */
//              .channel_num = 0; /* currently, only channel 0 was used */
//              .frame_rate = 0; /* don't set this parameter */
//              .play_speed = 0; /* normal speed */
//      };

CPB_STREAM_TYPE vid_stream_types[CPB_CHANNEL_NUM] = {	/* we only support 2 video channels */
	STREAM_MPEG2, STREAM_MPEG2
};

static struct proc_dir_entry *video_proc_entry = NULL;

static volatile unsigned int *vidmips_base = NULL;
static volatile unsigned int *orion_video_base = NULL;
#if defined(CONFIG_ARCH_ORION_CSM1200)
static volatile unsigned long *disp_base = NULL;
#endif
static volatile unsigned int *vid_userdata_base = NULL;

static unsigned int ud_block_size = 0;

void video_write(unsigned int val, unsigned int addr)
{
	video_writel(val,addr);
}

unsigned int video_read(unsigned int addr)
{
	return video_readl(addr);
}

DEFINE_SPINLOCK(orion_video_lock);
static DECLARE_WAIT_QUEUE_HEAD(timecode_wait_queue);
static DEFINE_SPINLOCK(timecode_lock);

static struct {
	unsigned int CPBDIR0L;
	unsigned int CPBDIR0U;
	unsigned int CPBDIR1L;
	unsigned int CPBDIR1U;
	unsigned int DPB0BASE;
	unsigned int DPB1BASE;

	unsigned int PPB0L;
	unsigned int PPB0U;
	unsigned int PPB1L;
	unsigned int PPB1U;

	unsigned int IPREDLBBASE;
	unsigned int MVLBBASE;
	unsigned int LFLBBASE;
	unsigned int MVBUF0BASE;
	unsigned int MVBUF1BASE;

	unsigned int FWBUF0BASE;
	unsigned int FWBUF0END;
	unsigned int FWBUF1BASE;
	unsigned int FWBUF1END;

	unsigned int MIPSINSL;

	unsigned int UD_REGION;
	unsigned int UD_SIZE;
	unsigned int UD_BLOCK_SIZE;
} vid_buf_conf;

#define VIB_RST_ADDR 0x10171200
void __VIB_Reset(void)
{
	unsigned int reg;

	reg = *vib_reset;//ReadReg32(VIB_RST_ADDR,&reg);
	reg = reg & (~(1<<10));
	*vib_reset = reg;//WriteReg32(VIB_RST_ADDR,reg);
	udelay(10);
	do
	{
		reg = *vib_reset;//ReadReg32(VIB_RST_ADDR,&reg);
	} while(reg & 0x400);
	reg = reg | 0x400;
	*vib_reset = reg;//WriteReg32(VIB_RST_ADDR,reg);
	udelay(10);
}

#define div(x) (x /16)
#define mod(x) (x % 16)
static void __video_get_pscancrop(struct vid_rect * rect)
{
        unsigned int regval = 0;
        unsigned int horizontal_size = 0;
        unsigned int vertical_size = 0;
        int tempcrop = 0;

        if(STREAM_H264 == vid_stream_types[CPB_CHANNEL_0]){
                regval = video_readl(HOST_IF_VID_MIPS_MAILBOX_10);
                rect->left = (regval >>24) & 0xff;
                rect->right = (regval >> 16) & 0xff;
                rect->top = (regval >> 8) & 0xff;
                rect->bottom = regval & 0xff;
        }
        else if(STREAM_MPEG2 == vid_stream_types[CPB_CHANNEL_0]){
                regval= video_readl(HOST_IF_VID_MIPS_MAILBOX_1);
                horizontal_size = (regval & 0x0fffc000)>>14;
                vertical_size = regval & 0x00003fff;
                if ((horizontal_size * 100 / vertical_size) <= 133) {
                            tempcrop = (vertical_size - (horizontal_size * 3 / 4)) / 2;
                            if (mod(tempcrop) > 10) {
                                    rect->top = 16 * (div(tempcrop) + 1);
                            }
                            else {
                                    rect->top = 16 * div(tempcrop);
                            }
                            rect->bottom = rect->top;
                            rect->left = 0;
                            rect->right = 0;
                    }
                    else if ((horizontal_size * 100 / vertical_size) > 133) {
                            tempcrop = (horizontal_size - (vertical_size * 4 / 3)) / 2;
                            if (mod(tempcrop) > 10) {
                                    rect->left = 16 * (div(tempcrop) + 1);
                            }
                            else {
                                    rect->left = 16 * div(tempcrop);
                            }
                            rect->right = rect->left;
                            rect->top = 0;
                            rect->bottom = 0;
                    }
        }
        else{/*other not support*/
            memset(rect,0,sizeof(struct vid_rect));
        }
}

static unsigned int __video_get_aspectratio(void)
{
        unsigned int regval;

        regval = video_readl(HOST_IF_VID_MIPS_MAILBOX_0);
        if((regval>>28) == 1){/*mpeg2*/
                return regval;/*low 4 bits show the aspect ratio*/
        }
        else if((regval>>28) == 2){/*h.264*/
                if((regval&0xff) == 0xff){/*custom*/
                        regval = video_readl(HOST_IF_VID_MIPS_MAILBOX_1);
                        return regval|0x20000000;
                }
                else{/*standard*/
                        return regval;
                }
        }
	else if((regval>>28) == 3){/*mpeg1*/
		return regval;
	}
        else{/*other not support*/
                return 0;/*unknown aspect ratio*/
        }
}

static void __video_pfm_reset(void)
{
        video_writel(CPB0_DIR_REGION, HOST_IF_VID_CPBDIRL0);
        video_writel(CPB0_DIR_REGION + CPB0_DIR_SIZE, HOST_IF_VID_CPBDIRU0);
        xport_writel(0x400, CPB0_DIR_REGION>>3);
        video_writel(CPB0_DIR_REGION, HOST_IF_VID_CPBDIRRP0);
}

static void __video_get_pts(long long *vid_pts)
{
	unsigned int regval = 0;

	unsigned int upval=0,lowval=0,stc=0;;
	
	regval = video_readl(HOST_IF_VID_MIPS_MAILBOX_15);
/*get stc*/
	upval = xport_readl(0x80);
	lowval= xport_readl(0x84);
	stc = (upval<<22)|(lowval>>10);
/*get diff*/
// __audio_readl(0x41212418);
/*calculate current pts*/
 	regval = stc - 320*45 - __audio_readl(0x41212418);

	__put_user(regval, (long long *) vid_pts);
	
#ifdef DRV_VIDEO_DEBUG
	printk("pts get 0x%x , stc = 0x%x, aud_pts = 0x%x\n", regval,stc,vid_pts);
#endif
}

static void __video_set_timecode(union video_time_code __user * timecode)
{
	unsigned int regval = 0;

	__get_user(regval, (unsigned int __user *) timecode);

        if(timecode->bits.reserved)
                vid_flags.bits.timecode_status = 1;
        else
                vid_flags.bits.timecode_status = 0;

	video_writel(regval, HOST_IF_VID_MIPS_MAILBOX_6);
#ifdef DRV_VIDEO_DEBUG
	printk("time code set 0x%x \n", regval);
#endif
}

static void __video_get_timecode(union video_time_code __user * timecode)
{
	unsigned int regval = 0;

	regval = video_readl(HOST_IF_VID_MIPS_MAILBOX_5);

	__put_user(regval & 0x1ffffff, (unsigned int __user *) timecode);
#ifdef DRV_VIDEO_DEBUG
	printk("time code get 0x%x \n", regval);
#endif
}

static void __video_set_ddr_conf(void)
{
	unsigned int vid_sta;

	vid_buf_conf.CPBDIR0L = CPB0_DIR_REGION;
	vid_buf_conf.CPBDIR0U = (CPB0_DIR_REGION + CPB0_DIR_SIZE);
	vid_buf_conf.CPBDIR1L = CPB1_DIR_REGION;
	vid_buf_conf.CPBDIR1U = (CPB1_DIR_REGION + CPB1_DIR_SIZE);
	vid_buf_conf.DPB0BASE = DPB0_REGION;
	vid_buf_conf.DPB1BASE = DPB1_REGION;

	vid_buf_conf.PPB0L = PPB0_REGION;
	vid_buf_conf.PPB0U = (PPB0_REGION + PPB0_SIZE);
	vid_buf_conf.PPB1L = PPB1_REGION;
	vid_buf_conf.PPB1U = (PPB1_REGION + PPB1_SIZE);

	vid_buf_conf.IPREDLBBASE = IPREDLB_REGION;
	vid_buf_conf.MVLBBASE = MVLB_REGION;
	vid_buf_conf.LFLBBASE = LFLB_REGION;
	vid_buf_conf.MVBUF0BASE = MVBUF0_REGION;
	vid_buf_conf.MVBUF1BASE = MVBUF1_REGION;

	vid_buf_conf.FWBUF0BASE = FWBUF0_REGION;
	vid_buf_conf.FWBUF0END = FWBUF0_REGION + FWBUF0_SIZE;
	vid_buf_conf.FWBUF1BASE = FWBUF1_REGION;
	vid_buf_conf.FWBUF1END = FWBUF1_REGION + FWBUF1_SIZE;

	vid_buf_conf.MIPSINSL = (unsigned int)vidmips_base;

	vid_buf_conf.UD_REGION = VIDEO_USER_DATA_REGION;
	vid_buf_conf.UD_SIZE = VIDEO_USER_DATA_SIZE;
	vid_buf_conf.UD_BLOCK_SIZE = ud_block_size;
	/* enable video VLD */
	vid_sta = video_readl(HOST_IF_VID_STA);
	if (vid_sta & 0x80000000)
		video_writel(0x40000000, HOST_IF_VID_CMD);

	/* configure the video decoder's DDR buffer and pointer */
	/* channel 0 */
	video_writel(vid_buf_conf.CPBDIR0U, HOST_IF_VID_CPBDIRU0);
	video_writel(vid_buf_conf.CPBDIR0L, HOST_IF_VID_CPBDIRL0);
	video_writel(vid_buf_conf.CPBDIR0L, HOST_IF_VID_CPBDIRRP0);
	video_writel(vid_buf_conf.DPB0BASE, HOST_IF_VID_DPB0);

	/* dpb 0/1 region & size configuration */
        video_writel(DPB0_REGION, HOST_IF_VID_DPB0);
        video_writel(DPB0_REGION, HOST_IF_VID_DPB0);
	video_writel(DPB0_SIZE, HOST_IF_VID_MIPS_MAILBOX_11);
        video_writel(DPB0_SIZE, HOST_IF_VID_MIPS_MAILBOX_11);
#if 0/*firmware not support DPB1 configuration*/
        video_writel(DPB1_REGION, HOST_IF_VID_DPB1);
        video_writel(DPB1_REGION, HOST_IF_VID_DPB1);
        video_writel(DPB1_SIZE, HOST_IF_VID_MIPS_MAILBOX_10);
        video_writel(DPB1_SIZE, HOST_IF_VID_MIPS_MAILBOX_10);
#endif
        video_writel(vid_buf_conf.PPB0L, HOST_IF_PLUTO_PPBL0_ADDR);
	video_writel(vid_buf_conf.PPB0U, HOST_IF_PLUTO_PPBU0_ADDR);
	video_writel(vid_buf_conf.MVBUF0BASE, HOST_IF_PLUTO_MVBUF0_ADDR);
	video_writel(vid_buf_conf.FWBUF0BASE, HOST_IF_PLUTO_FW_BASE_ADDR0);
	video_writel(vid_buf_conf.FWBUF0END, HOST_IF_PLUTO_FW_END_ADDR0);

	/* channel 1 */
	video_writel(vid_buf_conf.CPBDIR1U, HOST_IF_VID_CPBDIRU1);
	video_writel(vid_buf_conf.CPBDIR1L, HOST_IF_VID_CPBDIRL1);
	video_writel(vid_buf_conf.CPBDIR1L, HOST_IF_VID_CPBDIRRP1);
	video_writel(vid_buf_conf.DPB1BASE, HOST_IF_VID_DPB1);
	video_writel(vid_buf_conf.PPB1L, HOST_IF_PLUTO_PPBL1_ADDR);
	video_writel(vid_buf_conf.PPB1U, HOST_IF_PLUTO_PPBU1_ADDR);
	video_writel(vid_buf_conf.MVBUF1BASE, HOST_IF_PLUTO_MVBUF1_ADDR);
	video_writel(vid_buf_conf.FWBUF1BASE, HOST_IF_PLUTO_FW_BASE_ADDR1);
	video_writel(vid_buf_conf.FWBUF1END, HOST_IF_PLUTO_FW_END_ADDR1);

	/* comm register */
	video_writel(vid_buf_conf.IPREDLBBASE, HOST_IF_PLUTO_IPREDLB_ADDR);
	video_writel(vid_buf_conf.MVLBBASE, HOST_IF_PLUTO_MVLB_ADDR);
	video_writel(vid_buf_conf.LFLBBASE, HOST_IF_PLUTO_LFLB_ADDR);

	/* user data */
	video_writel(vid_buf_conf.UD_REGION, HOST_IF_VID_UDB);
	memset((unsigned int *)vid_userdata_base, 0, (vid_buf_conf.UD_SIZE) >> 2);
	*vid_userdata_base = MEM_SWAP32(vid_buf_conf.UD_REGION+vid_buf_conf.UD_BLOCK_SIZE);
	*(vid_userdata_base+2) = MEM_SWAP32(vid_buf_conf.UD_SIZE);
	*(vid_userdata_base+3) = MEM_SWAP32(vid_buf_conf.UD_BLOCK_SIZE);

	/* enable video decoder hardware */
	video_writel(1, HOST_IF_VID_ENA);

	return;
}

static int __video_firmware_to_mips(CPB_STREAM_TYPE type)
{
	size_t firmware_size_txt = 0, firmware_size_dat = 0;
	int i;
	unsigned int *p_src = NULL,*p_src_dat = NULL;
	volatile unsigned int *p_buf = NULL,*p_buf_dat = NULL;
	volatile unsigned int *start_addr = NULL;
	//change
	if ( (STREAM_MPEG2 == type) && (video_text_fw_mpeg2!=(struct firmware*)NULL)){
		firmware_size_txt= video_text_fw_mpeg2->size>>2;  // size/4
		p_src= (unsigned int *)video_text_fw_mpeg2->data; 
		firmware_size_dat = (video_data_fw_mpeg2->size)>>2;
		p_src_dat= (unsigned int *)video_data_fw_mpeg2->data;
	}
	else if ( (STREAM_H264 == type) && (video_text_fw_h264!=(struct firmware*)NULL)){
		firmware_size_txt= video_text_fw_h264->size>>2;
		p_src= (unsigned int *)video_text_fw_h264->data;	
		firmware_size_dat = (video_data_fw_h264->size)>>2;
		p_src_dat= (unsigned int *)video_data_fw_h264->data;
	}
	else if ((STREAM_VIB == type) && (video_text_fw_vib!=(struct firmware*)NULL)){	
		firmware_size_txt= video_text_fw_vib->size>>2;
		p_src= (unsigned int *)video_text_fw_vib->data;
		firmware_size_dat = (video_data_fw_vib->size)>>2;
		p_src_dat= (unsigned int *)video_data_fw_vib->data;
	}

	if(p_src!=NULL){
		//load txt
		start_addr = (volatile unsigned int *) vid_buf_conf.MIPSINSL;

		for (i = INST_OUT_MEM_SIZE / sizeof(unsigned int); i < firmware_size_txt; i++)
			*start_addr++ = p_src[i];
		for (i = 0; i < 4; i++)
			*start_addr++ = 0;

		/* --------------------- load instructions to IRAM --------------------- */
		p_buf = orion_video_base + MIPS_ISRAM_OFFSET;

		if (firmware_size_txt > (INST_OUT_MEM_SIZE + INST_MIPS_MEM_SIZE) / sizeof(unsigned int))
			firmware_size_txt = (INST_OUT_MEM_SIZE + INST_MIPS_MEM_SIZE) / sizeof(unsigned int);

		for (i = 0; i < firmware_size_txt; i++)
			p_buf[i] = p_src[i];
		
		//load data
		p_buf_dat= orion_video_base + MIPS_DSRAM_OFFSET;
		for (i = 0; i < firmware_size_dat; i++)
		{
			p_buf_dat[i] = p_src_dat[i];
		}
		for (; i < MIPS_DSRAM_MEM_SIZE / sizeof(unsigned int); i++)
			p_buf_dat[i] = 0x0;
	}
	return 0;
}

static int __video_set_streamtype(int cpb_channel, CPB_STREAM_TYPE type)
{
	switch (cpb_channel) {
	case CPB_CHANNEL_0:
		video_writel(type, HOST_IF_VID_CPB0_TYPE);
		break;
	case CPB_CHANNEL_1:
		video_writel(type, HOST_IF_VID_CPB1_TYPE);
		break;

	default:
		return -1;
	}

	return 0;
}

/*
static int __video_get_streamtype(int cpb_channel, CPB_STREAM_TYPE * type)
{
	if (NULL == type)
		return -1;

	switch (cpb_channel) {
	case CPB_CHANNEL_0:
		*type = video_readl(HOST_IF_VID_CPB0_TYPE) & 0x00000003;
		break;
	case CPB_CHANNEL_1:
		*type = video_readl(HOST_IF_VID_CPB1_TYPE) & 0x00000003;
		break;

	default:
		return -1;
	}

	return 0;
}

static int __video_set_params(union video_config_params conf)
{
	video_writel(conf.val, HOST_IF_VID_MIPS_STA1);

	return 0;
}

static int __video_get_params(union video_config_params *conf)
{
	if (NULL == conf)
		return -1;

	conf->val = video_readl(HOST_IF_VID_MIPS_STA1);

	return 0;
}
*/
static int __video_get_format(union video_format_params *fmt)
{
	unsigned int val;

	if (NULL == fmt)
		return -1;
#if 1
	val = video_readl(0x2d);

	fmt->bits.pic_height = val & 0x3fff;
	fmt->bits.pic_width = (val >> 14) & 0x3fff;
#else
	if (STREAM_H264 == vid_stream_types[CPB_CHANNEL_0]) {
		val = video_readl(0x2d);	/* FIXME@zhongkai's ugly code */

		fmt->bits.pic_height = val & 0x3fff;
		fmt->bits.pic_width = (val >> 14) & 0x3fff;
	}
	else {
		val = video_readl(0x2d);	/* FIXME@zhongkai's ugly code */

		fmt->bits.pic_height = val & 0x3fff;
		fmt->bits.pic_width = (val >> 12) & 0x3fff;
	}
#endif
	fmt->bits.src_frame_rate = (val >> 28) & 0xff;

	return 0;
}

#if defined(CONFIG_ARCH_ORION_CSM1201)
static int __video_get_src_format(union video_format_params *fmt)
{
	unsigned int val;

	if (NULL == fmt)
		return -1;

	val = video_readl(0x24);
	fmt->bits.pic_height = val & 0x3fff;
	fmt->bits.pic_width = (val >> 14) & 0x3fff;
	fmt->bits.src_frame_rate = (val >> 28) & 0xff;

	return 0;
}
#endif

static int __video_ctrl_cmd(unsigned int cmd, unsigned int params)
{
	int i = 0;
	if(params == 0){
		video_writel(cmd|0x80000000, VID_CMD_REG); 
	}
	else{
		cmd = (params<<8) | cmd;
		video_writel(cmd|0x80000000, VID_CMD_REG);
	}

	do{ 
		if(video_readl(VID_CMD_REG) >> 31 == 0x0)
			break;
		mdelay(2);
		i++;
		if (100 == i)
			break; 
	}while(1);

	//if(i >= 30)
		//printk("--------- if too many, pls tell me: bin.sun@celestialsemi.cn -------> kernel ctrl cmd 0x%08x  \n",cmd);

	return 0;
}
 
static int __video_enable_mips(int codec_type)
{

//	mdelay(50);
	video_writel(0, HOST_IF_VID_MIPS_STA0);	/* clean the status of mips */	
	video_writel(1, HOST_IF_VID_COP_MASK);	/* open all of intrs */
	/* write b18=1 to conf regs for fixing a h.264 bug which it smash the picture while switch program */
	//if (STREAM_H264 == vid_stream_types[CPB_CHANNEL_0])
		//video_writel(video_readl(HOST_IF_VID_MIPS_STA1) | 0x00040000, HOST_IF_VID_MIPS_STA1);

	if (gdbinfo_flags)
		printk(" %s:%d, HOST_IF_VID_MIPS_STA1 = %08x. \n", __FUNCTION__, __LINE__,
		       video_readl(HOST_IF_VID_MIPS_STA1));

	/* setting stream type ??? */
	__video_set_streamtype(CPB_CHANNEL_0, codec_type);

	/* enable mips to access core registers */
	video_writel(MIPS_ACCESS_VIDEO_REG, HOST_IF_VID_CMD_AE);

	/* start mips */
	video_writel(VIDMIPS_REGION /*vid_buf_conf.MIPSINSL */  >> 3, MIPS_RESET_OFFSET);
	if (gdbinfo_flags)
		printk(" %s:%d, HOST_IF_VID_MIPS_STA1 = %08x. \n", __FUNCTION__, __LINE__,
		       video_readl(HOST_IF_VID_MIPS_STA1));

	return 0;
}

static int __video_init(int codec_type)
{
	unsigned int mips_status;
	int running = 0, update_running = 0;
//	mdelay(10);
	video_writel(0x0, HOST_IF_VID_MIPS_STA1);
	mips_status = video_readl(MIPS_RESET_OFFSET);
	if (!(mips_status & (1 << 25))) {	/* mips is running, we must stop it */
		video_writel(1, HOST_IF_VID_COP_MASK);	/* open all of intrs */
		video_writel(1, HOST_IF_VID_COP_INT);	/* triger a init interrupt for mips */

		do {		/* we check whether the mips stopped */
			if (0x0001004e == video_readl(VID_MIPS_SAT_REG))
				break;
			else
				udelay(20);
		} while (1);

		/* disable the DDR port of mips */
		ddr_port_disable();

		running = video_readl(HOST_IF_VID_MIPS_MAILBOX_12);

		do
		{
		    udelay(50);
		    update_running = video_readl(HOST_IF_VID_MIPS_MAILBOX_12);
		    
		    if (running == update_running)
		    {
			break;
		    }
		    else
		    {
			running = update_running;
		    }
		    
		}while(1);

		/* reset mips */
		video_writel(0xffffffff, MIPS_RESET_OFFSET);
		udelay(5);	// lixun

		/* enable the DDR port of mips */
		ddr_port_enable();
	}
	__video_set_ddr_conf();
	video_writel(HOST_ACCESS_VIDEO_REG, HOST_IF_VID_CMD_AE);
	__video_firmware_to_mips(codec_type);
//	udelay(300);
	video_writel(0, VID_CMD_REG);
        __video_enable_mips(codec_type);

	running = video_readl(HOST_IF_VID_MIPS_STA1);
	video_writel(running|0x6000000, HOST_IF_VID_MIPS_STA1);
//	udelay(200);
	return 0;
}

/* 
 * the implementations of display functions
 */

static int __video_disp_init(void)
{
#if defined(CONFIG_ARCH_ORION_CSM1200)
	disp_base = (unsigned long *) ioremap(ORION_DISP_BASE, ORION_DISP_SIZE);
	if (NULL == disp_base)
		return -1;;
#endif
	return 0;
}
static inline void __load_firmware(void)
{
		int ret;
		ret = request_firmware(&video_text_fw_mpeg2, "video_mpeg2_text.bin", &(video_pdev->dev));
		if (unlikely(ret != 0 || video_text_fw_mpeg2== NULL)) {
			printk(KERN_ERR "Failed to load 1MP2 firmware code section\n");
		}
		else{ //load firmware successfully hr adds
			if (unlikely(video_text_fw_mpeg2->size > VIDMIPS_SIZE))
			{//firmware_size is wrong
					release_firmware(video_text_fw_mpeg2);
					video_text_fw_mpeg2=NULL;
					printk(KERN_ERR "Failed to load 2MP2 firmware code section\n");
			}
			if(video_text_fw_mpeg2!=NULL){
				ret = request_firmware(&video_data_fw_mpeg2, "video_mpeg2_data.bin", &(video_pdev->dev));
				if (unlikely(ret != 0 || video_data_fw_mpeg2== NULL)) {
					release_firmware(video_text_fw_mpeg2);
					video_text_fw_mpeg2=NULL;
					printk(KERN_ERR "Failed to load 1MP2 firmware data section\n");
				}
				else{
					if (unlikely(video_data_fw_mpeg2->size > VIDMIPS_SIZE ))
					{
				    		release_firmware(video_data_fw_mpeg2);
							video_data_fw_mpeg2=NULL;
							release_firmware(video_text_fw_mpeg2);
							video_text_fw_mpeg2=NULL;
							printk(KERN_ERR "Failed to load 2MP2 firmware data section\n");
					}
		         }
			}
			
		}

		ret = request_firmware(&video_text_fw_h264, "video_h264_text.bin", &(video_pdev->dev));
		if (unlikely(ret != 0 || video_text_fw_h264== NULL)) {
				printk(KERN_ERR "Failed to load 1H264 firmware code section\n");
		}
		else{
			if (unlikely(video_text_fw_h264->size > VIDMIPS_SIZE ))
			{
				release_firmware(video_text_fw_h264);
				video_text_fw_h264 = NULL;
				printk(KERN_ERR "Failed to load 2H264 firmware code section\n");
			}
			if(video_text_fw_h264!=NULL){
				ret = request_firmware(&video_data_fw_h264, "video_h264_data.bin", &(video_pdev->dev));
				if (unlikely(ret != 0 || video_data_fw_h264== NULL)) {
						release_firmware(video_text_fw_h264);
						video_text_fw_h264 = NULL;
						printk(KERN_ERR "Failed to load 1H264 firmware data section\n");
				}
				else{
					if (unlikely(video_data_fw_h264->size > MIPS_DSRAM_MEM_SIZE ))
					{
				    		release_firmware(video_data_fw_h264);
							video_data_fw_h264=NULL;
							release_firmware(video_text_fw_h264);
							video_text_fw_h264 = NULL;
							printk(KERN_ERR "Failed to load 2H264 firmware data section\n");
					}
				}
				}
		}

		ret = request_firmware(&video_text_fw_vib, "video_vib_text.bin", &(video_pdev->dev));
		if (unlikely(ret != 0 || video_text_fw_vib== NULL)) {
			printk(KERN_ERR "Failed to load 1VIB firmware code section\n");
		}
		else{
			if (unlikely(video_text_fw_vib->size > VIDMIPS_SIZE )){
				release_firmware(video_text_fw_vib);
				video_text_fw_vib=NULL;
				printk(KERN_ERR "Failed to load 2VIB firmware code section\n");
			}
			if(video_text_fw_vib!=NULL){
				ret = request_firmware(&video_data_fw_vib, "video_vib_data.bin", &(video_pdev->dev));
				if (unlikely(ret != 0 || video_data_fw_vib== NULL)) {
					release_firmware(video_text_fw_vib);
					video_text_fw_vib=NULL;
					printk(KERN_ERR "Failed to load 1VIB firmware data section\n");
				
				}
				else{
					if ( unlikely(video_data_fw_vib->size > MIPS_DSRAM_MEM_SIZE ))
					{
				    		release_firmware(video_data_fw_vib);
							video_data_fw_vib=NULL;
							release_firmware(video_text_fw_vib);
							video_text_fw_vib=NULL;
							printk(KERN_ERR "Failed to load 2VIB firmware data section\n");
					}
				}
				}
		}
		return;
}

static void __video_disp_release(void)
{
#if defined(CONFIG_ARCH_ORION_CSM1200)
	iounmap((void *) disp_base);
#endif
}

static int __video_disp_enable(int en_flags)
{
#if defined(CONFIG_ARCH_ORION_CSM1200)
	disp_base[DF_UPDATE_REG] = 0x1;
	disp_base[DF_DISP_ENABLE] = en_flags ? 0x1 : 0x0;
	disp_base[DF_UPDATE_REG] = 0x0;
#endif
	return 0;
}

#if defined(CONFIG_ARCH_ORION_CSM1200)
static int __video_layer_on(int layer, unsigned int on_flags)
{
	disp_base[DF_UPDATE_REG] = 0x1;
	disp_base[vid_regs_lst[layer][_VID_EN]] = on_flags ? 1 : 0;
	disp_base[DF_UPDATE_REG] = 0x0;

	return 0;
}
#endif

#if defined(CONFIG_ARCH_ORION_CSM1200)

static int __video_disp_alpha(int layer, unsigned int alpha)
{

	disp_base[DF_UPDATE_REG] = 0x1;
	disp_base[vid_regs_lst[layer][_VID_ALPHA]] = alpha & 0xff;
	disp_base[DF_UPDATE_REG] = 0x0;

	return 0;
}
#endif

static int __video_disp_src_fmt(int layer, unsigned int fmt)
{
#if defined(CONFIG_ARCH_ORION_CSM1200)
	disp_base[DF_UPDATE_REG] = 0x1;
	disp_base[vid_regs_lst[layer][_USR_CTRL]] = fmt ? disp_base[vid_regs_lst[layer][_USR_CTRL]] | (0x1 << 6) :	/* H.264 */
	disp_base[vid_regs_lst[layer][_USR_CTRL]] & ~(0x1 << 6);
	disp_base[DF_UPDATE_REG] = 0x0;
#endif
	return 0;
}

#if defined(CONFIG_ARCH_ORION_CSM1200)
static int __video_disp_position(int layer, struct vid_output_pos *pos)
{
	unsigned int reg = 0;

	disp_base[DF_UPDATE_REG] = 0x1;

	disp_base[vid_regs_lst[layer][_VID_SRC_X_CROP]] = (pos->src.right - pos->src.left + 1) << 16 | pos->src.left;
	disp_base[vid_regs_lst[layer][_VID_SRC_Y_CROP]] = (pos->src.bottom - pos->src.top + 1) << 16 | pos->src.top;

	disp_base[vid_regs_lst[layer][_VID_X_START]] = (pos->dst.left & ~0xf);	/* 16pixels aligned */
	disp_base[vid_regs_lst[layer][_VID_X_END]] = (pos->dst.right + 1) & ~0x1;	/* 2pixels aligned */
	disp_base[vid_regs_lst[layer][_VID_Y_START]] = (pos->dst.top & ~0xf);	/* 16pixels aligned */
	disp_base[vid_regs_lst[layer][_VID_Y_END]] = (pos->dst.bottom + 1) & ~0x1;	/* 2pixels aligned */

	reg = disp_base[vid_regs_lst[layer][_VID_Y_END]] - disp_base[vid_regs_lst[layer][_VID_Y_START]];
	reg |= (disp_base[vid_regs_lst[layer][_VID_X_END]] - disp_base[vid_regs_lst[layer][_VID_X_START]])<<16;
	video_writel(reg,HOST_IF_VID_MIPS_MAILBOX_7);

	if ((pos->dst.bottom - pos->dst.top) > 600) // to fix chroma bug in 720P mode, to use 1 tap filter for vertical
		disp_base[DF_VERTICAL_SEL] = 0x2020;    // 2-tap vertial scalar only at even Y line for more than 600 lines
	else
		disp_base[DF_VERTICAL_SEL] = 0x2222;    // 2-tap vertical scalar for less than 600 lines

	disp_base[DF_UPDATE_REG] = 0x0;

	return 0;
}
#endif

static int orion_video_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned long flags;
        unsigned int tmp_val = 0;
        struct vid_rect rect;
	struct vid_output_pos out_pos;
	union video_format_params vid_fmt;

	switch (cmd) {
		/* configure parameters */
	case CSVID_IOC_INPUT_MODE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_INPUT_MODE--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
                vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
		vid_configs.bits.stream_input_mode = arg;
		video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_STREAM_TYPE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_STREAM_TYPE--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		//vid_configs.bits.decode_mode = arg;
		if (arg != vid_stream_types[CPB_CHANNEL_0]) {
			vid_stream_types[CPB_CHANNEL_0] = arg;	/* FIXME@, only support one channel */
		}

		if (STREAM_H264 == arg) {
			__video_disp_src_fmt(0, 1 /* H.264 */ );
		}
		else
			__video_disp_src_fmt(0, 0 /* MPEG2 */ );
              __video_init(vid_stream_types[CPB_CHANNEL_0]);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_DECODER_MODE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_DECODER_MODE--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
                vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
		vid_configs.bits.decode_mode = arg;
		video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

    case CSVID_IOC_TRICK_MDOE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_INPUT_MODE--\n");
		#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
    		vid_configs.bits.trick_mode = arg;
		video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
		spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

	case CSVID_IOC_PLAY_SPEED:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_INPUT_MODE--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
                vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
    		vid_configs.bits.play_speed = arg;
		video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_FRAME_RATE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_FRAME_RATE--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
                vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
		vid_configs.bits.frame_rate = arg;
		video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

		/* control cmd */
	case CSVID_IOC_RESUME:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_RESUME--\n");
		#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __video_ctrl_cmd(VID_CMD_RESUME, 0);
                spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_PLAY:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_PLAY--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		__video_ctrl_cmd(VID_CMD_START, 0);	/* if not running, we start it */
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_PAUSE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_PAUSE--\n");
		#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __video_ctrl_cmd(VID_CMD_PAUSE, 0);
                spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_FREEZE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_FREEZE--\n");
		#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __video_ctrl_cmd(VID_CMD_FREEZE, 0);
                spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_STOP:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_STOP--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
                __video_ctrl_cmd(VID_CMD_STOP, 0);
                __video_init(vid_stream_types[CPB_CHANNEL_0]);
               spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_STEP:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_STEP--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
                __video_ctrl_cmd(VID_CMD_STEP, 0);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_SKIP:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_SKIP--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		/* do something here ... ... */
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_PTS_ENABLE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_PTS_ENABLE--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
		vid_configs.bits.pts_enable = 1;
		video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_PTS_DISABLE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_PTS_DISABLE--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
                vid_configs.bits.pts_enable = 0;
		video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
                spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_DISP_ON:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_DISP_ON--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
#if defined(CONFIG_ARCH_ORION_CSM1201)
			__df_update_start();
			DFVideoEna(__g_videolayer_index, 1); // video layer 0
			__df_update_end();
#else
		__video_layer_on(0, 1);
#endif
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_DISP_OFF:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_DISP_OFF--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
#if defined(CONFIG_ARCH_ORION_CSM1201)
			__df_update_start();
			DFVideoEna(__g_videolayer_index, 0); // video layer 0
			__df_update_end();
#else
		__video_layer_on(0, 0);
#endif
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_DISP_POS:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_DISP_POS--\n");
		#endif
		if (copy_from_user(&out_pos, (void *) arg, sizeof(struct vid_output_pos)))
			return -EFAULT;

		spin_lock_irqsave(&orion_video_lock, flags);
#if defined(CONFIG_ARCH_ORION_CSM1201)
		{
			unsigned int i = 0;
			unsigned int changemode_ack = 0;
			unsigned char is_timeout = 0;
			unsigned int alpha = 0;
			//unsigned int df_blanklevel = 0;
#if !defined(DEFAULT_SCALER)
			unsigned int scaler_src_width = 0, scaler_src_height = 0,scaler_des_width = 0,scaler_des_height = 0;
#endif
			if(1 == DFVideoIsDispWinChange(__g_videolayer_index, out_pos.dst, out_pos.src)){
				return 0;
			}
			else if(2 == DFVideoIsDispWinChange(__g_videolayer_index, out_pos.dst, out_pos.src)){
				is_timeout = 1;
			}

			changemode_ack = video_readl(HOST_IF_VID_MIPS_MAILBOX_3);
			while((changemode_ack>>29)&0x7){
				mdelay(4);
				i++;
				if (10 == i){
					video_writel(changemode_ack&0x1fffffff,HOST_IF_VID_MIPS_MAILBOX_3);
#ifdef DRV_VIDEO_DEBUG
					printk("KERNEL VIDEO ERROR1: Timeout\n");
#endif
					is_timeout = 1;
					break;
				}
				changemode_ack = video_readl(HOST_IF_VID_MIPS_MAILBOX_3);
			}

			if(is_timeout == 0){
				video_writel(((out_pos.dst.right - out_pos.dst.left)<<16) | (out_pos.dst.bottom - out_pos.dst.top),HOST_IF_VID_MIPS_MAILBOX_7);
				video_writel(changemode_ack|0x80000000,HOST_IF_VID_MIPS_MAILBOX_3);
				changemode_ack = video_readl(HOST_IF_VID_MIPS_MAILBOX_3);
				i = 0;
				while(((changemode_ack>>29)&0x7) != 0x2){
					mdelay(10);
					i++;
					if (60 == i){
						changemode_ack = video_readl(HOST_IF_VID_MIPS_MAILBOX_3);
						changemode_ack &= 0x1fffffff; 
						video_writel(changemode_ack|0x60000000,HOST_IF_VID_MIPS_MAILBOX_3);
#ifdef DRV_VIDEO_DEBUG
						printk("KERNEL VIDEO ERROR2: Timeout\n");
#endif
						is_timeout = 1;
						break;
					}
					changemode_ack = video_readl(HOST_IF_VID_MIPS_MAILBOX_3);
				}
			}
			mdelay(50);
			__df_update_start();
#if defined(DEFAULT_SCALER)
			DFVideoSetDefaultCoeff(__g_videolayer_index);
#else
			scaler_src_height = out_pos.src.bottom;
			scaler_src_width = out_pos.src.right;
			if((out_pos.src.right == 0)||(out_pos.src.bottom == 0)){
				i = video_readl(0x2d);
				scaler_src_height = i & 0x3fff;
				scaler_src_width = (i>>14) & 0x3fff;
			}
			scaler_des_height = out_pos.dst.bottom;
			scaler_des_width = out_pos.dst.right;
			DFSetVideoScalerCfg(__g_videolayer_index, scaler_src_width, scaler_src_height, scaler_des_width, scaler_des_height);
#endif
			DFVideoSetSrcCrop(__g_videolayer_index, out_pos.src);
			DFVideoSetDispWin(__g_videolayer_index, out_pos.dst);
			__df_update_end();

			if(is_timeout == 0){
				changemode_ack = video_readl(HOST_IF_VID_MIPS_MAILBOX_3);
				video_writel(changemode_ack|0x60000000,HOST_IF_VID_MIPS_MAILBOX_3);
			}
			else{
				changemode_ack = video_readl(HOST_IF_VID_MIPS_MAILBOX_3);
				video_writel(changemode_ack|0x60000000,HOST_IF_VID_MIPS_MAILBOX_3);
			}
		}
#else
		__video_disp_position(0, &out_pos);
#endif
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_DISP_ALPHA:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_DISP_ALPHA--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
#if defined(CONFIG_ARCH_ORION_CSM1201)
			__df_update_start();
			DFVideoSetLayerAlpha(__g_videolayer_index, arg, 0x00, 0x00); // setting alpha of video layer only.
			__df_update_end();
#else
		__video_disp_alpha(0, arg);
#endif
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_DISP_BLANK:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_DISP_BLANK--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		/* do something here ... ... */
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_GET_VIDEOFMT:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_GET_VIDEOFMT--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		__video_get_format(&vid_fmt);
		if (copy_to_user((void *) arg, (void *) &vid_fmt, sizeof(vid_fmt))) {
			spin_unlock_irqrestore(&orion_video_lock, flags);
			return -EFAULT;
		}
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

#if defined(CONFIG_ARCH_ORION_CSM1201)
	case CSVID_IOC_GET_SRC_VIDEOFMT:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_GET_SRC_VIDEOFMT--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		__video_get_src_format(&vid_fmt);
		if (copy_to_user((void *) arg, (void *) &vid_fmt, sizeof(vid_fmt))) {
			spin_unlock_irqrestore(&orion_video_lock, flags);
			return -EFAULT;
		}
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;
#endif

	case CSVID_IOC_SET_TIMECODE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_SET_TIMECODE--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		__video_set_timecode((union video_time_code __user *) arg);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_GET_PTS:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_GET_PTS--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		__video_get_pts((long long *) arg);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_GET_TIMECODE:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_GET_TIMECODE--\n");
		#endif
		spin_lock_irqsave(&orion_video_lock, flags);
		__video_get_timecode((union video_time_code __user *) arg);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_GET_NOTIFY_TYPE:
		{
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_GET_NOTIFY_TYPE--\n");
			#endif
			int tempval = 0;
                     spin_lock_irqsave(&timecode_lock, flags);
			if (vid_flags.bits.timecode_irq) {
				tempval |= 0x1;
				vid_flags.bits.timecode_status = 0;
                            vid_flags.bits.timecode_irq = 0;
			}
			if (vid_flags.bits.decodeerror_irq) {
				tempval |= 0x2;
				vid_flags.bits.decodeerror_status = 1;
                            vid_flags.bits.decodeerror_irq = 0;
			}
                     if (vid_flags.bits.aspectratio_irq) {
				tempval |= 0x4;
				vid_flags.bits.aspectratio_status = 1;
                            vid_flags.bits.aspectratio_irq = 0;
			}
                     if (vid_flags.bits.pscancrop_irq) {
				tempval |= 0x8;
				vid_flags.bits.pscancrop_status = 1;
                            vid_flags.bits.pscancrop_irq = 0;
			}
#if defined(CONFIG_ARCH_ORION_CSM1201)
			if (vid_flags.bits.sync_irq2 == 1) {
				tempval |= 0x10;
				vid_flags.bits.sync_status = 1;
                            vid_flags.bits.sync_irq2 = 0;
			}
			if(vid_flags.bits.sync_irq == 1){
				tempval |= 0x20;
				vid_flags.bits.sync_status = 1;
                            vid_flags.bits.sync_irq = 0;
			}
#else
			if (vid_flags.bits.sync_irq) {
				tempval |= 0x10;
				vid_flags.bits.sync_status = 1;
                            vid_flags.bits.sync_irq = 0;
			}
#endif
			if (vid_flags.bits.underflow_report_irq) {
				tempval |= 0x40;
				vid_flags.bits.underflow_report_status = 1;
				vid_flags.bits.underflow_report_irq = 0;
			}

			if (vid_flags.bits.formatchange_report_irq) {
				tempval |= 0x80;
				vid_flags.bits.formatchange_report_status = 1;
				vid_flags.bits.formatchange_report_irq = 0;
			}

			if (vid_flags.bits.srcformatchange_report_irq) {
				tempval |= 0x100;
				vid_flags.bits.srcformatchange_report_status = 1;
				vid_flags.bits.srcformatchange_report_irq = 0;
			}

#ifdef DRV_VIDEO_DEBUG
			printk(" kernel : poll value %d\n", tempval);
#endif
			__put_user(tempval, (unsigned int __user *) arg);
			spin_unlock_irqrestore(&timecode_lock, flags);
			break;
		}

        case CSVID_IOC_SET_ERROR_LEVEL:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_SET_ERROR_LEVEL--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
		vid_configs.bits.error_report_level = arg;
                if(arg == 0){
                        vid_flags.bits.decodeerror_status = 0;
                }
                else{
                        vid_flags.bits.decodeerror_status = 1;
                }
		video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

	case CSVID_IOC_SET_UNDERFLOW:
		spin_lock_irqsave(&orion_video_lock, flags);
		vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
		vid_configs.bits.underflow_report = arg;
		if(arg == 0){
		    vid_flags.bits.underflow_report_status = 0;
		}
		else{
		    vid_flags.bits.underflow_report_status = 1;
		}
		video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
		spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

	case CSVID_IOC_SET_FORMATCHANGE_REPORT:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_SET_FORMATCHANGE_REPORT--\n");
		#endif
		spin_lock_irqsave(&timecode_lock, flags);
		if(1 == arg)
			vid_flags.bits.formatchange_report_status = 1;
		else
			vid_flags.bits.formatchange_report_status = 0;

		vid_flags.bits.formatchange_report_irq = 0;
		spin_unlock_irqrestore(&timecode_lock, flags);
		break;

	case CSVID_IOC_SET_SRCFORMATCHANGE_REPORT:
		#ifdef DRV_VIDEO_TESTCTL
		printk("\n--CSVID_IOC_SET_SRCFORMATCHANGE_REPORT--\n");
		#endif
		spin_lock_irqsave(&timecode_lock, flags);
		if(1 == arg)
			vid_flags.bits.srcformatchange_report_status = 1;
		else
			vid_flags.bits.srcformatchange_report_status = 0;

		vid_flags.bits.srcformatchange_report_irq = 0;
		spin_unlock_irqrestore(&timecode_lock, flags);
		break;

        case CSVID_IOC_PMF_RESET:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_PMF_RESET--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __video_pfm_reset();
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
        case CSVID_IOC_PMF_GETCPB_ADDR:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_PMF_GETCPB_ADDR--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __put_user(CPB0_REGION, (unsigned int __user *) arg);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
        case CSVID_IOC_PMF_GETCPB_SIZE:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_PMF_GETCPB_SIZE--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __put_user(CPB0_SIZE, (unsigned int __user *) arg);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
        case CSVID_IOC_PMF_GETDIR_ADDR:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_PMF_GETDIR_ADDR--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __put_user(CPB0_DIR_REGION, (unsigned int __user *) arg);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
        case CSVID_IOC_PMF_GETDIR_SIZE:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_PMF_GETDIR_SIZE--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __put_user(CPB0_DIR_SIZE, (unsigned int __user *) arg);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
         case CSVID_IOC_PMF_SET_DIR_WP:
		 	#ifdef DRV_VIDEO_TESTCTL
		 	printk("\n--CSVID_IOC_PMF_SET_DIR_WP--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __get_user(tmp_val,(unsigned int __user * )arg);
                xport_writel(0x400,tmp_val);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
         case CSVID_IOC_PMF_GET_DIR_WP:
		 	#ifdef DRV_VIDEO_TESTCTL
		 	printk("\n--CSVID_IOC_PMF_GET_DIR_WP--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __put_user(xport_readl(0x400),(unsigned int __user * )arg);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
         case CSVID_IOC_PMF_SET_DIR_RP:
		 	#ifdef DRV_VIDEO_TESTCTL
		 	printk("\n--CSVID_IOC_PMF_SET_DIR_RP--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __get_user(tmp_val,(unsigned int __user * )arg);
                xport_writel(0x404,tmp_val);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
        case CSVID_IOC_PMF_GET_DIR_RP:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_PMF_GET_DIR_RP--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __put_user(xport_readl(0x404),(unsigned int __user * )arg);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
        case CSVID_IOC_PMF_SET_CPBDIRU0:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_PMF_SET_CPBDIRU0--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __get_user(tmp_val,(unsigned int __user * )arg);
                video_writel(tmp_val, HOST_IF_VID_CPBDIRU0);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
         case CSVID_IOC_PMF_GET_CPBDIRU0:
		 	#ifdef DRV_VIDEO_TESTCTL
		 	printk("\n--CSVID_IOC_PMF_GET_CPBDIRU0--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __put_user(video_readl(HOST_IF_VID_CPBDIRU0),(unsigned int __user * )arg);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
         case CSVID_IOC_PMF_SET_CPBDIRL0:
		 	#ifdef DRV_VIDEO_TESTCTL
		 	printk("\n--CSVID_IOC_PMF_SET_CPBDIRL0--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __get_user(tmp_val,(unsigned int __user * )arg);
                video_writel(tmp_val, HOST_IF_VID_CPBDIRL0);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;
         case CSVID_IOC_PMF_GET_CPBDIRL0:
		 	#ifdef DRV_VIDEO_TESTCTL
		 	printk("\n--CSVID_IOC_PMF_GET_CPBDIRL0--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __put_user(video_readl(HOST_IF_VID_CPBDIRL0),(unsigned int __user * )arg);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

         case CSVID_IOC_SET_ASPECTRATIO_STATUS:
		 	#ifdef DRV_VIDEO_TESTCTL
		 	printk("\n--CSVID_IOC_SET_ASPECTRATIO_STATUS--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                if(1 == arg)
                    vid_flags.bits.aspectratio_status = 1;
                else
                    vid_flags.bits.aspectratio_status = 0;
                
                vid_flags.bits.aspectratio_irq = 0;
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

         case CSVID_IOC_SET_PSCANCROP_STATUS:
		 	#ifdef DRV_VIDEO_TESTCTL
				printk("\n--CSVID_IOC_SET_PSCANCROP_STATUS--\n");
			#endif
                spin_lock_irqsave(&timecode_lock, flags);
                if(1 == arg)
                    vid_flags.bits.pscancrop_status = 1;
                else
                    vid_flags.bits.pscancrop_status = 0;
                
                vid_flags.bits.pscancrop_irq = 0;
                spin_unlock_irqrestore(&timecode_lock, flags);
                break;

         case CSVID_IOC_SET_SYNC_STATUS:
		 	#ifdef DRV_VIDEO_TESTCTL
		 		printk("\n--CSVID_IOC_SET_SYNC_STATUS--\n");
			#endif
                spin_lock_irqsave(&timecode_lock, flags);
                if(1 == arg)
                    vid_flags.bits.sync_status = 1;
                else
                    vid_flags.bits.sync_status = 0;
                
                vid_flags.bits.sync_irq = 0;
		  vid_flags.bits.sync_irq2 = 0;
                spin_unlock_irqrestore(&timecode_lock, flags);
                break;

            case CSVID_IOC_GET_PSCANCROP:
				#ifdef DRV_VIDEO_TESTCTL
				printk("\n--CSVID_IOC_GET_PSCANCROP--\n");
				#endif
                spin_lock_irqsave(&timecode_lock, flags);
                __video_get_pscancrop(&rect);
                copy_to_user((void __user *)arg, (void *)&rect, sizeof(struct vid_rect));
                spin_unlock_irqrestore(&timecode_lock, flags);
                break;

            case CSVID_IOC_GET_ASPECTRATIO:
				#ifdef DRV_VIDEO_TESTCTL
				printk("\n--CSVID_IOC_GET_ASPECTRATIO--\n");
				#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                __put_user(__video_get_aspectratio(), (unsigned int __user * )arg);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

            case CSVID_IOC_SET_FORCE_3TO2_POLLDOW:
				#ifdef DRV_VIDEO_TESTCTL
				printk("\n--CSVID_IOC_SET_FORCE_3TO2_POLLDOW--\n");
				#endif
                spin_lock_irqsave(&orion_video_lock, flags);
                vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
		vid_configs.bits.force_32_poll_down_flag = arg;
		video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

            case CSVID_IOC_SET_NOWAIT_SYNC:
				#ifdef DRV_VIDEO_TESTCTL
				printk("\n--CSVID_IOC_SET_NOWAIT_SYNC--\n");
				#endif
			spin_lock_irqsave(&orion_video_lock, flags);
			vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
			vid_configs.bits.sync_flag = arg & 0x1;  /* 0 for no wait sync:normal mode, 1 for wait sync, 2 for no wait sync:fast mode */
			vid_configs.bits.nosyncwait_fast_mode = (arg >> 1) & 0x1;
			video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
			spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

		case CSVID_IOC_VIB_CONFIG:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_VIB_CONFIG--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
		  __get_user(tmp_val,(unsigned int __user * )arg);
                video_writel(tmp_val, VID_VIB_PARA_REG);
#if defined(CONFIG_ARCH_ORION_CSM1201)
		orion_gpio2_register_module_set("VIB",1);
 		(*pin_mux0_addr) |= (1 << 13);
		udelay(10);
 		(*gpio_pin_mux1_addr) &= ~(0x1ff << 14);
		udelay(10);
#endif
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

		case CSVID_IOC_VIB_RESET:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_SET_USER_DATA_STATUS--\n");
			#endif
                spin_lock_irqsave(&orion_video_lock, flags);
		  __VIB_Reset();
                spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

		case CSVID_IOC_SET_USER_DATA_STATUS:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_SET_USER_DATA_STATUS--\n");
			#endif
			spin_lock_irqsave(&orion_video_lock, flags);
			vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
			vid_configs.bits.ud_stauts = arg;
			video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
			spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

		case CSVID_IOC_GET_USERDATA_ADDR:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_GET_USERDATA_ADDR--\n");
			#endif
			spin_lock_irqsave(&orion_video_lock, flags);
			__put_user(VIDEO_USER_DATA_REGION, (unsigned int __user * )arg);
			spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

		case CSVID_IOC_GET_USERDATA_SIZE:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_GET_USERDATA_SIZE--\n");
			#endif
			spin_lock_irqsave(&orion_video_lock, flags);
			__put_user(VIDEO_USER_DATA_SIZE, (unsigned int __user * )arg);
			spin_unlock_irqrestore(&orion_video_lock, flags);
                break;

		case CSVID_IOC_SET_USERDATA_BLOCK_SIZE:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_SET_USERDATA_BLOCK_SIZE--\n");
			#endif
			spin_lock_irqsave(&orion_video_lock, flags);
			__get_user(ud_block_size, (unsigned int __user * )arg);
			spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

		case CSVID_IOC_SET_ERROR_SKIP_MODE:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_SET_ERROR_SKIP_MODE--\n");
			#endif
			spin_lock_irqsave(&orion_video_lock, flags);
			vid_configs.val = video_readl(HOST_IF_VID_MIPS_STA1);
			vid_configs.bits.error_ignore_mode = arg;
			video_writel(vid_configs.val, HOST_IF_VID_MIPS_STA1);
			spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

#if defined(CONFIG_ARCH_ORION_CSM1201)
		case CSVID_IOC_DISP_POS_BEIYANG:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_DISP_POS_BEIYANG--\n");
			#endif
			if (copy_from_user(&out_pos, (void *) arg, sizeof(struct vid_output_pos)))
			return -EFAULT;

			spin_lock_irqsave(&orion_video_lock, flags);
			if(1 == DFVideoIsDispWinChange(1, out_pos.dst, out_pos.src)){
				return 0;
			}
			__df_update_start();
			DFVideoSetDefaultCoeff(1);
			DFVideoSetSrcCrop(1, out_pos.src);
			DFVideoSetDispWin(1, out_pos.dst);
			__df_update_end();
		break;

		case CSVID_IOC_DISP_ALPHA_BEIYANG:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_DISP_ALPHA_BEIYANG--\n");
			#endif
			spin_lock_irqsave(&orion_video_lock, flags);
			__df_update_start();
			DFVideoSetLayerAlpha(1, arg, 0x00, 0x00); // setting alpha of video layer only.
			__df_update_end();
			spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

		case CSVID_IOC_SET_STARTDELAY:
			#ifdef DRV_VIDEO_TESTCTL
			printk("\n--CSVID_IOC_SET_STARTDELAY--\n");
			#endif
			spin_lock_irqsave(&orion_video_lock, flags);
			arg &= 0xfffff;
			__video_ctrl_cmd(VID_CMD_SET_STARTDALAY, arg);	/* if not running, we start it */
			spin_unlock_irqrestore(&orion_video_lock, flags);
		break;

#endif
		default:
    		break;
	}

	return 0;
}

/*
 * Architectures vary in how they handle caching for addresses
 * outside of main memory.
 *
 */
static inline int uncached_access(struct file *file, unsigned long addr)
{
#if defined(__i386__)
	/*
	 * On the PPro and successors, the MTRRs are used to set
	 * memory types for physical addresses outside main memory,
	 * so blindly setting PCD or PWT on those pages is wrong.
	 * For Pentiums and earlier, the surround logic should disable
	 * caching for the high addresses through the KEN pin, but
	 * we maintain the tradition of paranoia in this code.
	 */
	if (file->f_flags & O_SYNC)
		return 1;
 	return !( test_bit(X86_FEATURE_MTRR, boot_cpu_data.x86_capability) ||
		  test_bit(X86_FEATURE_K6_MTRR, boot_cpu_data.x86_capability) ||
		  test_bit(X86_FEATURE_CYRIX_ARR, boot_cpu_data.x86_capability) ||
		  test_bit(X86_FEATURE_CENTAUR_MCR, boot_cpu_data.x86_capability) )
	  && addr >= __pa(high_memory);
#elif defined(__x86_64__)
	/* 
	 * This is broken because it can generate memory type aliases,
	 * which can cause cache corruptions
	 * But it is only available for root and we have to be bug-to-bug
	 * compatible with i386.
	 */
	if (file->f_flags & O_SYNC)
		return 1;
	/* same behaviour as i386. PAT always set to cached and MTRRs control the
	   caching behaviour. 
	   Hopefully a full PAT implementation will fix that soon. */	   
	return 0;
#elif defined(CONFIG_IA64)
	/*
	 * On ia64, we ignore O_SYNC because we cannot tolerate memory attribute aliases.
	 */
	return !(efi_mem_attributes(addr) & EFI_MEMORY_WB);
#else
	/*
	 * Accessing memory above the top the kernel knows about or through a file pointer
	 * that was marked O_SYNC will be done non-cached.
	 */
	if (file->f_flags & O_SYNC)
		return 1;
	return addr >= __pa(high_memory);
#endif
}
static int orion_video_open(struct inode *inodp,struct file *filp)
{
	__load_firmware();
	return 0;
}

static int orion_video_mmap(struct file *filp, struct vm_area_struct *vma)
{
#if defined(__HAVE_PHYS_MEM_ACCESS_PROT)
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;

	vma->vm_page_prot = phys_mem_access_prot(file, offset,
						 vma->vm_end - vma->vm_start,
						 vma->vm_page_prot);
#elif defined(pgprot_noncached)
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	int uncached;

	uncached = uncached_access(filp, offset);
	if (uncached)
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff,
			    vma->vm_end-vma->vm_start,
			    vma->vm_page_prot))
		return -EAGAIN;
	return 0;
}

static int orion_video_release(struct inode *inodp,struct file *filp)
{
	//add something,release firmware
	if(video_text_fw_mpeg2!=NULL)
		release_firmware(video_text_fw_mpeg2);
	if(video_text_fw_h264!=NULL)
		release_firmware(video_text_fw_h264);
	if(video_text_fw_vib!=NULL)
		release_firmware(video_text_fw_vib);
	if(video_data_fw_mpeg2!=NULL)
		release_firmware(video_data_fw_mpeg2);
	if(video_data_fw_h264!=NULL)
		release_firmware(video_data_fw_h264);
	if(video_data_fw_vib!=NULL)
		release_firmware(video_data_fw_vib);
	return 0;
}

static ssize_t orion_video_write(struct file *file, const char __user * buffer, size_t len, loff_t * offset)
{
#ifdef DRV_VIDEO_DEBUG
	printk("%s run!\n", __FUNCTION__);
#endif
	//__video_filldatatoCPB((const unsigned int __user *) buffer, len);

	return len;
}

static unsigned int orion_video_poll(struct file *filp, poll_table * wait)
{
	unsigned int mask = 0;
#ifdef DRV_VIDEO_DEBUG
	//printk("video poll run!\n");
#endif
	if ((vid_flags.bits.timecode_irq) ||(vid_flags.bits.decodeerror_irq) ||(vid_flags.bits.aspectratio_irq) ||
		(vid_flags.bits.pscancrop_irq)||(vid_flags.bits.sync_irq)||(vid_flags.bits.sync_irq2)||
		(vid_flags.bits.underflow_report_irq)||(vid_flags.bits.formatchange_report_irq)||(vid_flags.bits.srcformatchange_report_irq)){
		mask = POLLIN | POLLWRNORM;
		return mask;
	}
	poll_wait(filp, &timecode_wait_queue, wait);

#ifdef DRV_VIDEO_DEBUG
        //printk("mask = 0x%x\n",mask);
#endif
        return mask;
}

static struct file_operations orion_video_fops = {
	.owner = THIS_MODULE,
	.open = orion_video_open,
	.ioctl = orion_video_ioctl,
	.mmap = orion_video_mmap,
	.release = orion_video_release,
	.write = orion_video_write,
	.poll = orion_video_poll
};

static struct miscdevice orion_video_miscdev = {
	MISC_DYNAMIC_MINOR,
	"orion_video",
	&orion_video_fops
};

static int video_proc_write(struct file *file, const char *buffer,unsigned long count, void *data)
{
	unsigned int addr;
	unsigned int val;

        int parser_timout = 0;
	int parser_err = 0;
	int pipe_timeout = 0;
	int pipe_err = 0;

	const char *cmd_line = buffer;

	if (strncmp("rl", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = video_readl(addr);
		printk(" readw [0x%04x] = 0x%08x \n", addr, val);
	}
	else if (strncmp("wl", cmd_line, 2) == 0) {
		addr = simple_strtol(&cmd_line[3], NULL, 16);
		val = simple_strtol(&cmd_line[7], NULL, 16);
		video_writel(val, addr);
	}
	else if (strncmp("gdbinfo", cmd_line, 7) == 0) {
		unsigned int data = video_readl(HOST_IF_VID_MIPS_MAILBOX_14);
		printk("[%08x] \n", data);
		printk("[VIDEO] CPB PTS NUM:      %d PARSER PTS NUM: %d\n", data >> 16, data & 0xFFFF);
		printk("[VIDEO] MAX DELAY TIME:   %d EXCEED 30ms CNT: %d\n", (data >> 16) / 45, data & 0xFFFF);
		printk("[VIDEO] CPB OVERFLOW CNT: %d \n", data);
		printk("[VIDEO] MustOutputNum:    %d \n", data);
		printk("[VIDEO] First PTS:        0x%08x \n", data);
		printk("[VIDEO] ReSendDFNUM:      %d DFRepeatNum: %d\n", data >> 16, data & 0xFFFF);
		printk("[VIDEO] PPB OVERFLOW:     %d CPB OVERFLOW: %d\n", data >> 16, data & 0xFFFF);
		printk("[VIDEO] Pipe Err:         (%d) Pipe Tout: (%d) Parser Err: (%d) Parser Tout: (%d)\n",
		       (data >> 24) & 0xFF, (data >> 16) & 0xFF, (data >> 8) & 0xFF, data & 0xFF);
		printk("[VIDEO] flag:             (%d) BufIdx: (%d) IsIPic: (%d) DfFlag: (%d)\n", (data >> 24) & 0xFF,
		       (data >> 16) & 0xFF, (data >> 8) & 0xFF, data & 0xFF);
		printk("[VIDEO] DF Size:          %d x %d\n", data >> 16, data & 0xFFFF);

		data = video_readl(HOST_IF_VID_MIPS_MAILBOX_15);
		printk("[%08x] \n\n", data);
		parser_timout = data & 0xFF;
		parser_err = (data >> 8) & 0xFF;
		pipe_timeout = (data >> 16) & 0xFF;
		pipe_err = (data >> 24) & 0xFF;
		printk("[VIDEO] parser_timout: %d parser_err: %d pipe_timeout: %d pipe_err: %d\n", parser_timout,
		       parser_err, pipe_timeout, pipe_err);
		printk("[VIDEO] ref miss num: %d  slice miss num: %d\n", data >> 16, data & 0xffff);

		printk("[%08x] \n", video_readl(0x20280 >> 2));
		printk("[%08x] \n", video_readl(0x20284 >> 2));
		printk("[%08x] \n", video_readl(0x20288 >> 2));
		printk("[%08x] \n", video_readl(0x2028c >> 2));
		printk("[%08x] \n", video_readl(0x20290 >> 2));
		printk("[%08x] \n", video_readl(0x20294 >> 2));
		printk("[%08x] \n", video_readl(0x20298 >> 2));
		printk("[%08x] \n", video_readl(0x2029c >> 2));
	}
	else if (strncmp("enable_gdb", cmd_line, 10) == 0)
		gdbinfo_flags = 1;
	else if (strncmp("disable_gdb", cmd_line, 11) == 0)
		gdbinfo_flags = 0;
	else if (strncmp("cpb", cmd_line, 3) == 0){
		printk("cpb start address 0x%x\n",CPB0_REGION);
	}

	return count;
}

struct orionfb_interrupt {
	wait_queue_head_t wait;
	unsigned int count;
	int is_display;
	struct fb_info *cur_fb_info;
};
extern struct orionfb_interrupt oriondf_1201_vblank_int[2];
extern void vbi_ttx_sending(unsigned long data);
extern unsigned int Is_TTX;
irqreturn_t orion_vid_irq(int irq, void *dev_id, struct pt_regs * egs)
{	
	unsigned long flag;
	//static int df0_count =0;
	//static int df1_count = 0;
	unsigned int irq_reg = 0;

#if defined(CONFIG_ARCH_ORION_CSM1201)
	unsigned int temp_val = 0;
#endif

#ifdef DRV_VIDEO_DEBUG
//	printk("video irq!\n");
#endif
	irq_reg = video_readl(HOST_IF_VID_MIPS_MAILBOX_14);
#ifdef DRV_VIDEO_DEBUG
//	printk("0x%x\n", irq_reg);
#endif

        if(irq_reg&VID_INT_CMDACCEPT){
#ifdef DRV_VIDEO_DEBUG
		printk("command accept!\n");
#endif
        }

	if(irq_reg&VID_INT_NEWVIDEOFORMAT){
#ifdef DRV_VIDEO_DEBUG
		printk("new video format!\n");
#endif
		spin_lock_irqsave(&timecode_lock,flag);
		if(vid_flags.bits.formatchange_report_status){
			vid_flags.bits.formatchange_report_irq = 1;
			wake_up(&timecode_wait_queue);
		}
		spin_unlock_irqrestore(&timecode_lock,flag);
       }

	if(irq_reg&VID_INT_NEWASPECTRATIO){
#ifdef DRV_VIDEO_DEBUG
		printk("new aspect ratio!\n");
#endif
                spin_lock_irqsave(&timecode_lock,flag);
                if((vid_flags.bits.aspectratio_status)||(vid_flags.bits.pscancrop_status)){
                    if(vid_flags.bits.aspectratio_status)vid_flags.bits.aspectratio_irq = 1;
                    if(vid_flags.bits.pscancrop_status)vid_flags.bits.pscancrop_irq = 1;
		    wake_up(&timecode_wait_queue);
                }
                spin_unlock_irqrestore(&timecode_lock,flag);
       }

	if(irq_reg&VID_INT_STARTSENDCMDTODF){
                spin_lock_irqsave(&timecode_lock,flag);
		  if(vid_flags.bits.sync_status){
#if defined(CONFIG_ARCH_ORION_CSM1201)
			temp_val = video_readl(HOST_IF_VID_MIPS_MAILBOX_3);
			temp_val &= 0x3;
			if(temp_val == 0x1){
				vid_flags.bits.sync_irq = 1;
		       	wake_up(&timecode_wait_queue);
			}
			else if(temp_val == 0x3){
				vid_flags.bits.sync_irq2 = 1;
		       	wake_up(&timecode_wait_queue);
			}
			else{
				vid_flags.bits.sync_irq2 = 1;
		       	wake_up(&timecode_wait_queue);
			}
#else
			vid_flags.bits.sync_irq = 1;
		       wake_up(&timecode_wait_queue);
#endif
                }
		spin_unlock_irqrestore(&timecode_lock,flag);
#ifdef DRV_VIDEO_DEBUG
		printk("kernel: start send command to DF!\n");
		printk("kernel: syncirq = %d, sync_irq2 = %d\n",vid_flags.bits.sync_irq,vid_flags.bits.sync_irq2);
#endif
       }

	if(irq_reg&VID_INT_NEWTIMECODE){
		spin_lock_irqsave(&timecode_lock,flag);
                if(vid_flags.bits.timecode_status){
		        vid_flags.bits.timecode_irq = 1;
		        wake_up(&timecode_wait_queue);
                }
		spin_unlock_irqrestore(&timecode_lock,flag);
#ifdef DRV_VIDEO_DEBUG
		printk("new time code!\n");
#endif
       }

	if(irq_reg&VID_INT_FINDUSERDATA){
#ifdef DRV_VIDEO_DEBUG
		printk("found user data!\n");
#endif
       }

	if(irq_reg&VID_INT_ADDFRMTOSWFIFO){
#ifdef DRV_VIDEO_DEBUG
		printk("AddFrmToSWFIFO!\n");
#endif
       }
 
	if(irq_reg&VID_INT_FINDDECODERERROR){
		spin_lock_irqsave(&timecode_lock,flag);
                if(vid_flags.bits.decodeerror_status){
		        vid_flags.bits.decodeerror_irq = 1;
		        wake_up(&timecode_wait_queue);
                }
		spin_unlock_irqrestore(&timecode_lock,flag);
#ifdef DRV_VIDEO_DEBUG
		printk("found decode error!\n");
#endif

       }

        if (irq_reg&VID_INT_UNDERFLOW){
	    spin_lock_irqsave(&timecode_lock,flag);
	    if(vid_flags.bits.underflow_report_status){
		vid_flags.bits.underflow_report_irq = 1;
		wake_up(&timecode_wait_queue);
	    }
	    spin_unlock_irqrestore(&timecode_lock,flag);
#ifdef DRV_VIDEO_DEBUG
	    printk("found video data underflow!\n");
#endif
	}

        if(irq_reg&VID_INT_M2VDRESET){
#ifdef DRV_VIDEO_DEBUG
		printk("VID_INT_M2VDRESET IN!\n");
#endif

        __video_init(vid_stream_types[CPB_CHANNEL_0]);
        udelay(10);
        __video_ctrl_cmd(VID_CMD_START, 0);
        
#ifdef DRV_VIDEO_DEBUG
		printk("VID_INT_M2VDRESET OUT!\n");
#endif
       }

	if(irq_reg&VID_INT_VIBERROR){
#ifdef DRV_VIDEO_DEBUG
		printk("VID_INT_VIBERROR!\n");
#endif
#if !defined(CONFIG_ARCH_ORION_CSM1201)
	__VIB_Reset();
	__video_ctrl_cmd(VID_CMD_START, 0);
#endif
	}

#if defined(CONFIG_ARCH_ORION_CSM1201)
	if(irq_reg&VID_INT_DF0){
		if(oriondf_1201_vblank_int[0].is_display == 1){
			oriondf_1201_vblank_int[0].is_display = 0;
			wake_up(&oriondf_1201_vblank_int[0].wait);
		}
		if(Is_TTX){
			static int ii = 0;
			if (ii == 50) {
				ii = 0;
			}
			ii++;

			vbi_ttx_sending(0);
		}
	
	}
	if(irq_reg&VID_INT_DF1){
		if(oriondf_1201_vblank_int[1].is_display == 1){
			oriondf_1201_vblank_int[1].is_display = 0;
			wake_up(&oriondf_1201_vblank_int[1].wait);
		}
		if(Is_TTX){
			static int ii = 0;
			if (ii == 50) {
				ii = 0;
			}
			ii++;

			vbi_ttx_sending(0);
		}
	}
	
	if(irq_reg&VID_INT_SRCFORMAT){
#ifdef DRV_VIDEO_DEBUG
		printk("VID_INT_SRCFORMAT!\n");
#endif
		spin_lock_irqsave(&timecode_lock,flag);
		if(vid_flags.bits.srcformatchange_report_status){
			vid_flags.bits.srcformatchange_report_irq = 1;
			wake_up(&timecode_wait_queue);
		}
		spin_unlock_irqrestore(&timecode_lock,flag);
	}
#endif

	{
		unsigned int temp_val = 0;
		temp_val = video_readl(HOST_IF_VID_MIPS_MAILBOX_14);
		temp_val = temp_val^irq_reg;
		video_writel(temp_val, HOST_IF_VID_MIPS_MAILBOX_14);
	}
	irq_reg = video_readl(HOST_IF_VID_HOST_INT);
	video_writel(irq_reg & 0xfffffffe, HOST_IF_VID_HOST_INT);

       video_writel(0x1, HOST_IF_VID_HOST_MASK);

	return IRQ_HANDLED;
}

int __init orion_video_init(void)
{
	int ret = 0;

	ret = -ENODEV;
	if (misc_register(&orion_video_miscdev))
		goto ERR_NODEV;
	
	/* for requesting firmware */
    	video_pdev = platform_device_register_simple("video_device", 0, NULL, 0);
    	if (IS_ERR(video_pdev)) {
        	return -ENODEV;
    	}
	
	ret = -EIO;
	if (!request_mem_region(ORION_VIDEO_BASE, ORION_VIDEO_SIZE, "Orion Video"))
		goto ERR_MEMREQ;

	ret = -EIO;
	if (NULL == (orion_video_base = ioremap(ORION_VIDEO_BASE, ORION_VIDEO_SIZE)))
		goto ERR_MEMMAP;

	ret = -EIO;
	if (NULL == (vidmips_base = ioremap(VIDMIPS_REGION, VIDMIPS_SIZE)))
		goto ERR_MEMMAP2;

	ret = -EIO;
       if (NULL == ( ddr_port = (unsigned long*)ioremap(DDR_CONTROL_MASK_REG, 4)))
		goto ERR_MEMMAP3;

	ret = -EIO;
	if (NULL == ( vib_reset = (unsigned long*)ioremap(0x10171200, 4)))
		goto ERR_MEMMAP4;

	ret = -EIO;
	if (NULL == ( vid_userdata_base = ioremap(VIDEO_USER_DATA_REGION, VIDEO_USER_DATA_SIZE)))
		goto ERR_MEMMAP5;

#if defined(CONFIG_ARCH_ORION_CSM1201)
	ret = -EIO;
	if (NULL == ( pin_mux0_addr = (unsigned long*)ioremap(0x10171400, 4)))
		goto ERR_MEMMAP6;

	ret = -EIO;
	if (NULL == ( gpio_pin_mux1_addr = (unsigned long*)ioremap(0x10260044, 4)))
		goto ERR_MEMMAP7;
#endif	   	

	if (request_irq(20, orion_vid_irq, SA_INTERRUPT, "cs_video", NULL)) {
		printk(KERN_ERR "csdrv_vid: cannot register IRQ \n");
		return -EIO;
	}
	/* initialize display */
{
	int irq_reg = 0;
	
	irq_reg = video_readl(HOST_IF_VID_HOST_MASK);
        video_writel(irq_reg | 0x1, HOST_IF_VID_HOST_MASK);
}
	__video_disp_init();
	__video_disp_enable(1);

	video_proc_entry = create_proc_entry("vid_io", 0, NULL);
	if (NULL != (void *)video_proc_entry) {		
		video_proc_entry->write_proc = &video_proc_write;
	}

	vid_configs.val = 0;
	vid_flags.val = 0;

	printk(KERN_INFO "%s: Orion Video driver was initialized, at address@[phyical addr = %08x, size = %x] \n",
	       "orion_video", ORION_VIDEO_BASE, ORION_VIDEO_SIZE);

	video_writel(0x0, HOST_IF_VID_MIPS_MAILBOX_3);
	return 0;

#if defined(CONFIG_ARCH_ORION_CSM1201)
	ERR_MEMMAP7:
	iounmap((void *) gpio_pin_mux1_addr);
	ERR_MEMMAP6:
	iounmap((void *) pin_mux0_addr);
#endif
	ERR_MEMMAP5:
	iounmap((void *) vib_reset);	
	ERR_MEMMAP4:
	iounmap((void *) ddr_port);	
	ERR_MEMMAP3:
	iounmap((void *) vidmips_base);	
      ERR_MEMMAP2:
	iounmap((void *) orion_video_base);
      ERR_MEMMAP:
	release_mem_region(ORION_VIDEO_BASE, ORION_VIDEO_SIZE);
      ERR_MEMREQ:
      ERR_NODEV:
	return ret;
}

static void __exit orion_video_exit(void)
{
	iounmap((void *) orion_video_base);
	iounmap((void *) vidmips_base);
       iounmap((void *) ddr_port); 
	iounmap((void *) vib_reset);	
	iounmap((void *) vid_userdata_base);	
#if defined(CONFIG_ARCH_ORION_CSM1201)
	iounmap((void *) pin_mux0_addr);
	iounmap((void *) gpio_pin_mux1_addr);
#endif
	release_mem_region(ORION_VIDEO_BASE, ORION_VIDEO_SIZE);
	if (NULL != video_proc_entry)
		remove_proc_entry("vid_io", NULL);

	misc_deregister(&orion_video_miscdev);
        __video_disp_release();
}

void orion_vid_disable_repeat_cmd(unsigned int outif_id)
{
	unsigned int temp_val = 0;
	
	temp_val = video_readl( HOST_IF_VID_MIPS_MAILBOX_3);
	if(outif_id == 0){
		temp_val |= 0x4;  
	}
	else if(outif_id == 1){
		temp_val |= 0x8;
	}
	else{
		temp_val |= 0xc;
	}
	video_writel(temp_val, HOST_IF_VID_MIPS_MAILBOX_3);
}

#if defined(CONFIG_ARCH_ORION_CSM1201)
EXPORT_SYMBOL(orion_vid_disable_repeat_cmd);
EXPORT_SYMBOL(vid_stream_types);
EXPORT_SYMBOL(__g_videolayer_index);
EXPORT_SYMBOL(video_read);
EXPORT_SYMBOL(video_write);
#endif
module_init(orion_video_init);
module_exit(orion_video_exit);
