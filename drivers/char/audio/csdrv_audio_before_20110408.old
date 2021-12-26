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
#include <linux/device.h>
#include <linux/firmware.h>

#include <linux/mem_define.h>

#if 0
#if defined(CONFIG_ARCH_ORION_CSM1201)
#include "1201/Audio_Cmd_LPCM.h"
#include "1201/Audio_Data_LPCM.h" 
#include "1201/Audio_Cmd_Mp2.h"
#include "1201/Audio_Data_Mp2.h"
#include "1201/Audio_Ext_Data_MP2.h"
#include "1201/Audio_Data_Dts.h"
#include "1201/Audio_Cmd_Dts.h"
#include "1201/Audio_Ext_Data_DTS.h"
#include "1201/Audio_Cmd_Ac3.h"
#include "1201/Audio_Data_Ac3.h"
#include "1201/Audio_Cmd_Aaclatm_Ddr.h"
#include "1201/Audio_Data_Aaclatm.h"
#include "1201/Audio_Cmd_Aacadts_Ddr.h"
#include "1201/Audio_Data_Aacadts.h"
#include "1201/Audio_Ext_Data_EAACV2.h"
#include "1201/Audio_Cmd_Aib.h"
#include "1201/Audio_Data_Aib.h"
#else
#include "Audio_Data_LPCM.h" 
#include "Audio_Cmd_LPCM.h"
#include "Audio_Cmd_Ac3.h"
#include "Audio_Cmd_Mp2.h"
//#include "Audio_Data_AAC.h"
#include "Audio_Data_Ac3.h"
#include "Audio_Data_Mp2.h"

/*dts firmware and coefficient table */
#include "Audio_Data_Dts.h"
#include "Audio_Cmd_Dts.h"

/* add by ying 20081007 */
#include "Audio_Cmd_Eac3.h"
#include "Audio_Data_Eac3.h"

#include "Audio_Cmd_Aacpart.h"
#include "Audio_Data_Aacpart.h"
#include "Audio_Cmd_Aaclatm.h"
#include "Audio_Data_Aaclatm.h"
#include "Audio_Cmd_Sbrpart.h"
#include "Audio_Data_Sbrpart.h"
#endif
#endif
//#include "Audio_external_data_dts.h"
//#include "Audio_external_data_aac.h"
//#include "Audio_EAC3_VQ.h"

//#define DRV_AUDIO_DEBUG
//#define DRV_AUDIO_INFO
//#define DRV_AUDIO_TESTINFO
#define FIX_MIX_POINTERS

#ifdef CONFIG_ARCH_ORION_CSM1201
#define SUPPORT_EAC3_1201
#endif

#define CAB_LOW_ADDR        	(CAB_REGION >> 4)
#define CAB_UP_ADDR     	((( CAB_REGION + CAB_SIZE ) >> 4) - 1)
#define CAB_WRPTR       	CAB_LOW_ADDR

#define PTS_AUDIO_LOW_ADDR      (AUD_PTS_REGION >> 4)
#define PTS_AUDIO_UP_ADDR       (((AUD_PTS_REGION + AUD_PTS_SIZE) >> 4) - 1)
#define PTS_AUDIO_WRPTR     	PTS_AUDIO_LOW_ADDR

#if defined(CONFIG_ARCH_ORION_CSM1201)
#define AUD_EXT_CODE_REGION AUD_REGION
#define AUD_EXT_CODE_SIZE   0x40000
#define AUD_EXT_DATA_REGION (AUD_REGION + AUD_EXT_CODE_SIZE)
#define AUD_EXT_DATA_SIZE   0x40000
#define AUD_MIX_REGION (AUD_EXT_DATA_REGION + AUD_EXT_DATA_SIZE)
#define AUD_MIX_SIZE (AUD_SIZE - 0x80000)
#else
/* added by wangxuewei 2008.03.14 */
/* --------------------------------------------------------------*/
#define AUD_DTS_TABLE_SIZE    	0x18000	//AUD_SIZE
/* ---------------------------------------------------------------*/

#define AUD_HEAAC_TABLE_SIZE    0x20000

#define AUD_EAC3_TABLE_SIZE    0x2CE0 //0x2CD0
/*fixme*/
#define AUD_MIX_REGION  	AUD_REGION
#define AUD_MIX_SIZE    	(AUD_SIZE - AUD_DTS_TABLE_SIZE - AUD_HEAAC_TABLE_SIZE)	//AUD_SIZE
/*end fixme*/

/*         audio mix |    DTS  0x18000  | HEAAC  0x20000   */

/* added by wangxuewei 2008.03.14 */
/* --------------------------------------------------------------*/
#define AUD_DTS_TABLE_REGION  	(AUD_MIX_REGION + AUD_MIX_SIZE)

#define AUD_HEAAC_TABLE_REGION  	(AUD_DTS_TABLE_REGION + AUD_DTS_TABLE_SIZE)
#define AUD_EAC3_TABLE_REGION     (AUD_HEAAC_TABLE_REGION) /* add by ying 20081007*/
#endif
/* ---------------------------------------------------------------*/
#define MIX_AUDIO_LOW_ADDR      (AUD_MIX_REGION >> 4)
#define MIX_AUDIO_UP_ADDR       (((AUD_MIX_REGION + AUD_MIX_SIZE) >> 4) - 1)
#define MIX_AUDIO_WRPTR     	MIX_AUDIO_LOW_ADDR
#define MIX_BUFFER_TOGGLE31     0x80000000
#define MIX_BUFFER_TOGGLE15     0x8000

MODULE_AUTHOR("Jia.ma (jia.ma@celestialsemi.cn)");
MODULE_DESCRIPTION("the celestial audio driver");
MODULE_VERSION("0.2");
MODULE_LICENSE("GPL");

/*MEM and REG Baseaddress Space Define/Alloc*/
//#define CSDRV_AUD_REG_BASE  0X10150000
#define CSDRV_AUD_REG_BASE  			0X41210000
#define CSDRV_AUD_REG_SIZE   			0X8000

#define CSDRV_IDCS_REG_BASE 			0x10170000
#define CSDRV_IDCS_REG_SIZE  			0x1500

#if defined(CONFIG_ARCH_ORION_CSM1201)
#define CSDRV_AUD_ITCM_BASE 			0x41220000  // by ahb bus: 32bit
#else
#define CSDRV_AUD_ITCM_BASE 			0x20000000  // by ahb bus: 32bit
#endif
#define CSDRV_AUD_ITCM_SIZE  			0x8000

#if defined(CONFIG_ARCH_ORION_CSM1201)
#define CSDRV_AUD_DTCM_BASE 			0x41230000  // by ahb bus: 32bit
#else
#define CSDRV_AUD_DTCM_BASE 			0x20008000  // by ahb bus: 32bit
#endif
#define CSDRV_AUD_DTCM_SIZE  			0x8000

#define CSDRV_AUD_RISC_REG  			0x41200000
#define CSDRV_AUD_RISC_REG_SIZE   		0x10

/*The audio register address define*/
#define AUD_RISC_CTRL                           (CSDRV_AUD_RISC_REG+0x00000)

#define AUD_MIX_TONE_SAMPNUM                    (CSDRV_AUD_REG_BASE+0x0000)
#define AUD_MIX_TONE_AMP                        (CSDRV_AUD_REG_BASE+0x0004)
#define AUD_MIX_TONE_MODE                       (CSDRV_AUD_REG_BASE+0x0008)

#define AUD_PTSUP                               (CSDRV_AUD_REG_BASE+0x1000)
#define AUD_PTSLOW                              (CSDRV_AUD_REG_BASE+0x1004)
#define AUD_PTSRP                               (CSDRV_AUD_REG_BASE+0x1008)

#define AUD_CABUP                               (CSDRV_AUD_REG_BASE+0x100C)
#define AUD_CABLOW                              (CSDRV_AUD_REG_BASE+0x1010)
#define AUD_CABRP                               (CSDRV_AUD_REG_BASE+0x1014)

#define AUD_ENABLE                              (CSDRV_AUD_REG_BASE+0x1018)
#define AUD_SHITCNT                             (CSDRV_AUD_REG_BASE+0x1038)
#define AUD_PTS_ENABLE                          (CSDRV_AUD_REG_BASE+0x104C)

#define AUD_I2SCNTL                             (CSDRV_AUD_REG_BASE+0x1050)
#define AUD_CABWP                               (CSDRV_AUD_REG_BASE+0x1058)
#define AUD_PTSWP                               (CSDRV_AUD_REG_BASE+0x1060)

#define AUD_OFIFO                               (CSDRV_AUD_REG_BASE+0x2000)
#define AUD_OFIFOCNT                            (CSDRV_AUD_REG_BASE+0x2004)

#define AUD_IFIFO                               (CSDRV_AUD_REG_BASE+0x2008)
#define AUD_IFIFOCNT                            (CSDRV_AUD_REG_BASE+0x200C)

#define AUD_ERRFIFO                             (CSDRV_AUD_REG_BASE+0x2010)
#define AUD_ERRFIFOCNT                          (CSDRV_AUD_REG_BASE+0x2014)

#define AUD_PTSFIFO                             (CSDRV_AUD_REG_BASE+0x2018)
#define AUD_PTSFIFOCNT                          (CSDRV_AUD_REG_BASE+0x201C)

#define AUD_STC                                 (CSDRV_AUD_REG_BASE+0x2020)

#define AUD_MAILBOX0                            (CSDRV_AUD_REG_BASE+0x2400)
#define AUD_MAILBOX1                            (CSDRV_AUD_REG_BASE+0x2404)
#define AUD_MAILBOX2                            (CSDRV_AUD_REG_BASE+0x2408)
#define AUD_MAILBOX3                            (CSDRV_AUD_REG_BASE+0x240C)
#define AUD_MAILBOX4                            (CSDRV_AUD_REG_BASE+0x2410)
#define AUD_MAILBOX5                            (CSDRV_AUD_REG_BASE+0x2414)

#define AUD2VID_MAILBOX                         (CSDRV_AUD_REG_BASE+0x2418)
#define VID2AUD_MAILBOX                         (CSDRV_AUD_REG_BASE+0x241C)

#define AUD_INT                                 (CSDRV_AUD_REG_BASE+0x2420)
#define AUD_DSP_INT_SRC                         (CSDRV_AUD_REG_BASE+0x2424)
#define AUD_DSP_INT_MASK                        (CSDRV_AUD_REG_BASE+0x2428)

#define AUD_INPUT_LEVEL                         (CSDRV_AUD_REG_BASE+0x242C)
#define AUD_INPUT_CTRL                          (CSDRV_AUD_REG_BASE+0x2430)
#define AUD_INPUT_CNT                           (CSDRV_AUD_REG_BASE+0x2434)
#define AUD_INPUT_SAMPLE                        (CSDRV_AUD_REG_BASE+0x2438)
#define AUD_INPUT_RATE                          (CSDRV_AUD_REG_BASE+0x243C)

#define AUD_DTCM_DMA_CTL                        (CSDRV_AUD_REG_BASE+0x3000)
#define AUD_DTCM_DMA_CMD                        (CSDRV_AUD_REG_BASE+0x3004)
#define AUD_DTCM_DMA_STAT                       (CSDRV_AUD_REG_BASE+0x3008)
#define AUD_DTCM_DMA_BASE_ADDR                  (CSDRV_AUD_REG_BASE+0x300C)

#define AUD_DTCM_DMA_CH0REM                     (CSDRV_AUD_REG_BASE+0x3010)
#define AUD_DTCM_DMA_CH1REM                     (CSDRV_AUD_REG_BASE+0x3014)
#define AUD_DTCM_DMA_CH2REM                     (CSDRV_AUD_REG_BASE+0x3018)
#define AUD_DTCM_DMA_CH3REM                     (CSDRV_AUD_REG_BASE+0x301C)

/* spdif register space */
#define AUD_SPDIF_VER                           (CSDRV_AUD_REG_BASE+0x4000)
#define AUD_CHANNEL_STAT1                       (CSDRV_AUD_REG_BASE+0x4004)
#define AUD_CHANNEL_STAT12                      (CSDRV_AUD_REG_BASE+0x4008)
#define AUD_CHANNEL_STAT2                       (CSDRV_AUD_REG_BASE+0x400C)
#define AUD_CHANNEL_STAT3                       (CSDRV_AUD_REG_BASE+0x4010)
#define AUD_CHANNEL_STAT4                       (CSDRV_AUD_REG_BASE+0x4014)
#define AUD_CHANNEL_STAT5                       (CSDRV_AUD_REG_BASE+0x4018)
#define AUD_CHANNEL_STAT6                       (CSDRV_AUD_REG_BASE+0x401C)
#define AUD_SPDIF_USERDATA1                     (CSDRV_AUD_REG_BASE+0x4020)
#define AUD_SPDIF_USERDATA2                     (CSDRV_AUD_REG_BASE+0x4024)
#define AUD_SPDIF_USERDATA3                     (CSDRV_AUD_REG_BASE+0x4028)
#define AUD_SPDIF_USERDATA4                     (CSDRV_AUD_REG_BASE+0x402C)
#define AUD_SPDIF_USERDATA5                     (CSDRV_AUD_REG_BASE+0x4030)
#define AUD_SPDIF_USERDATA6                     (CSDRV_AUD_REG_BASE+0x4034)
#define AUD_SPDIF_VALID                         (CSDRV_AUD_REG_BASE+0x4038)
#define AUD_SPDIF_I2S_CMD                       (CSDRV_AUD_REG_BASE+0x403C)
#define AUD_SPDIF_STAT                          (CSDRV_AUD_REG_BASE+0x4040)

#define AUD_DMA_BUF_BASE                        (CSDRV_AUD_REG_BASE+0x5000)
#define AUD_DMA_ADDR                            (CSDRV_AUD_REG_BASE+0x6000)
#define AUD_DMA_LEN                             (CSDRV_AUD_REG_BASE+0x6004)
#define AUD_DMA_CTRL                            (CSDRV_AUD_REG_BASE+0x6008)
#define AUD_DMA_STAT                            (CSDRV_AUD_REG_BASE+0x600C)

/*idcs rigster space */
#define AUD_FREQ_CTRL                           (CSDRV_IDCS_REG_BASE+0x1108)	// bit 9:1 enable
#define AUD_CLK_GEN_HPD                         (CSDRV_IDCS_REG_BASE+0x110C)
#define AUD_CLK_GEN_FREQ                        (CSDRV_IDCS_REG_BASE+0x1118)
#define AUD_CLK_GEN_JITTER_LO                   (CSDRV_IDCS_REG_BASE+0x111C)
#define AUD_CLK_GEN_JITTER_HI                   (CSDRV_IDCS_REG_BASE+0x1404)	// lower 3 bits
#define AUD_PLL_REG  				(CSDRV_IDCS_REG_BASE+0x1408)
#define IDCS_RESET_REG                          (CSDRV_IDCS_REG_BASE+0x1200)

//mailbox 1
#define AUD_ATYPE_INT                           0x00000001
#define AUD_BITRATE_INT                         0x00000002
#define AUD_SAMRATE_INT                         0x00000003
#define AUD_ACMOD_INT                           0x00000004
#define AUD_DECODED_BYTES_INT                   0x00000005
#define AUD_DECODED_FRAMES_INT                  0x00000006
#define AUD_MIX_INFO_INT                        0x00000040
#define OTHER_INT                               0x80000000
//mailbox 2
#define MUTE_SET                                0x00000001
#define DRC_SET                                 0x00000002
#define PTS_SET                                 0x00000004
#define SUR_SET                                 0x00000008
#define ACTIVE_SET                              0x00000010
#define SPDIF_SET                               0x00000020
#define PCMSPDIF_SET                            0x00000040//non pcm
#define MIX_SET                                 0x00000080
#define INPUT_MODE_SET                         0X00000200

#define VOLUME_SET                              0x00000800
#define OUTCH_SET                               0x00001000
#define PLAYMODE_SET                            0x00002000
#define STARTDELAY_SET                          0x00004000
#define DECODEDBYTE_SET                         0x00008000
#define EQUALIZE_SET                            0x00010000

#define ERROR_LEVEL_SET                     0x00020000
#define CHANNEL_BALANCE                     0x00040000

#if 0
#define INFO_REQ_AUDIO_TYPE                     0x00200000
#define INFO_REQ_BIT_RATE                       0x00400000
#define INFO_REQ_SAMPLE_RATE                    0x00800000
#define INFO_REQ_AC_MODE                        0x01000000
#define INFO_REQ_DECODEDBYTE                    0x02000000
#define INFO_REQ_DECODEDFRAME                   0x04000000
#else
#define REQINFO_SET			        0x00200000
#endif
#define OTHER_CMD_SET                           0x10000000
//mailbox 4
#define AUD_REQ_STARTDELAY      		0x00000001
#define AUD_REQ_DECODED_BYTES			0x00000002

#define AUD_REQ_EQ_GAIN        		0x00000004

#define AUD_REQ_DDR_BASE        		0x00001000
#define AUD_REQ_DDR_RP_WP       		0x00002000
#define AUD_REQ_DDR_BIT_SR_LEVEL_UP     	0x000004000

#define AUD_REQ_DDR_BASE_KALA       		0x000100000
#define AUD_REQ_DDR_RP_WP_KALA      		0x00020000
#define AUD_REQ_DDR_BIT_SR_LEVEL_UP_KALA        0x00040000

#define AUD_CHANNEL_BALANCE     0x00080000

#define AUD_WATCHDOG    			0xf0000001
#define AUD_SAMPLE_RATE_INT     		0xf0000002
#define AUD_MIX_READ_WP     			0xf0000003
#define AUD_MIX_WRITE_RP        		0xf0000004
#define AUD_ERROR       			0xffffffff

//mailbox 5
#define AUD_MIX_SR_ERROR        		0x00000001
#define AUD_MIX_NO_DATA_ERROR       		0x00000002
#define AUD_EQUA_MODE_ERROR     		0x00000004
#define AUD_KALAOK_SR_ERROR     		0x00000008
#define AUD_OUTPUT_CNANNEL_ERROR        	0x00000010
#define AUD_ERROR_LEVEL                         0x00000020

#define  CSDRV_AUDIO_IOC_MAGIC  'a'

// switching firmware
#define AUD_INT_REQUIRE_EAC3   0xf000000c
#define AUD_INT_REQUIRE_AC3    0xf000000d
#define AUD_INT_REQUIRE_AAC 0xf000000e
#define AUD_INT_REQUIRE_SBR 0xf000000f
//io command
#define  CSAUD_IOC_SET_DECTYPE        		_IOW(CSDRV_AUDIO_IOC_MAGIC, 1, int)
#define  CSAUD_IOC_SET_RESET          		_IOW(CSDRV_AUDIO_IOC_MAGIC, 2, int)
#define  CSAUD_IOC_SET_FREQ           		_IOW(CSDRV_AUDIO_IOC_MAGIC, 3, int)
#define  CSAUD_IOC_SET_DRC            		_IOW(CSDRV_AUDIO_IOC_MAGIC, 4, int)
#define  CSAUD_IOC_SET_MUTE           		_IOW(CSDRV_AUDIO_IOC_MAGIC, 5, int)
#define  CSAUD_IOC_SET_SURROUND       		_IOW(CSDRV_AUDIO_IOC_MAGIC, 6, int)
#define  CSAUD_IOC_SET_PTS_SYNC       		_IOW(CSDRV_AUDIO_IOC_MAGIC, 7, int)
//#define  CSAUD_IOC_SET_PCM_FROMAT     		_IOW(CSDRV_AUDIO_IOC_MAGIC, 8, int)
#define  CSAUD_IOC_SET_VOLUME         		_IOW(CSDRV_AUDIO_IOC_MAGIC, 9, int)
#define  CSAUD_IOC_SET_PLAY           		_IOW(CSDRV_AUDIO_IOC_MAGIC, 10, int)
#define  CSAUD_IOC_SET_STOP           		_IOW(CSDRV_AUDIO_IOC_MAGIC, 11, int)
//#define  CSAUD_IOC_SET_ACTIVE_MODE        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 12, int)
//#define  CSAUD_IOC_SET_OUTPUT_MODE       	_IOW(CSDRV_AUDIO_IOC_MAGIC, 13, int)
//#define  CSAUD_IOC_SET_OUTPUT_DATA_TYPE      	_IOW(CSDRV_AUDIO_IOC_MAGIC, 14, int)
#define  CSAUD_IOC_SET_MIXER_STATUS      	_IOW(CSDRV_AUDIO_IOC_MAGIC, 15, int)
#define  CSAUD_IOC_SET_MIXER_CONFIG      	_IOW(CSDRV_AUDIO_IOC_MAGIC, 16, int)
#define  CSAUD_IOC_SET_OUTPUT_CHANNEL        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 17, int)
#define  CSAUD_IOC_SET_PLAYMODE      		_IOW(CSDRV_AUDIO_IOC_MAGIC, 18, int)
#define  CSAUD_IOC_SET_STARTDELAY        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 19, int)
#define  CSAUD_IOC_SET_DECODEDBYTES      	_IOW(CSDRV_AUDIO_IOC_MAGIC, 20, int)
#define  CSAUD_IOC_SET_EQUALIZER_STATUS      	_IOW(CSDRV_AUDIO_IOC_MAGIC, 21, int)
#define  CSAUD_IOC_SET_EQUALIZER_CONFIG      	_IOW(CSDRV_AUDIO_IOC_MAGIC, 22, int)
#define  CSAUD_IOC_GET_DECTYPE        		_IOW(CSDRV_AUDIO_IOC_MAGIC, 23, int)
#define  CSAUD_IOC_GET_BITRATE       		_IOW(CSDRV_AUDIO_IOC_MAGIC, 24, int)
#define  CSAUD_IOC_GET_SAMPLERATE        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 25, int)
#define  CSAUD_IOC_GET_ACMODE        		_IOW(CSDRV_AUDIO_IOC_MAGIC, 26, int)
#define  CSAUD_IOC_GET_DECODEDBYTES        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 27, int)
#define  CSAUD_IOC_GET_DECODEDFRAME        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 28, int)
#define  CSAUD_IOC_GET_VOLUME        		_IOW(CSDRV_AUDIO_IOC_MAGIC, 29, int)
#define  CSAUD_IOC_GET_MIXER_CONFIG      	_IOW(CSDRV_AUDIO_IOC_MAGIC, 30, int)
#define  CSAUD_IOC_GET_EQUALIZER_CONFIG      	_IOW(CSDRV_AUDIO_IOC_MAGIC, 31, int)
#define  CSAUD_IOC_WRITE_MIXER_BUFFER        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 32, int)
#define  CSAUD_IOC_SET_OUTPUT_DEV        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 33, int)
#define  CSAUD_IOC_GET_OUTPUT_DEV        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 34, int)
#define  CSAUD_IOC_SET_I2SFORMAT     		_IOW(CSDRV_AUDIO_IOC_MAGIC, 35, int)
#define  CSAUD_IOC_GET_I2SFORMAT     		_IOW(CSDRV_AUDIO_IOC_MAGIC, 36, int)
#define CSAUD_IOC_SET_ERROR_LEVEL   _IOW(CSDRV_AUDIO_IOC_MAGIC, 37, int)
#define CSAUD_IOC_GET_ERROR_LEVEL   _IOW(CSDRV_AUDIO_IOC_MAGIC, 38, int)
#define CSAUD_IOC_GET_OUTPUT_CHANNEL    _IOW(CSDRV_AUDIO_IOC_MAGIC, 39, int)
#define CSAUD_IOC_PFM_GETCAB_ADDR   _IOW(CSDRV_AUDIO_IOC_MAGIC, 40, int)
#define CSAUD_IOC_PFM_GETCAB_SIZE   _IOW(CSDRV_AUDIO_IOC_MAGIC, 41, int)
#define CSAUD_IOC_PFM_GETCAB_RP   _IOW(CSDRV_AUDIO_IOC_MAGIC, 42, int)
#define CSAUD_IOC_PFM_GETCAB_WP   _IOW(CSDRV_AUDIO_IOC_MAGIC, 43, int)
#define CSAUD_IOC_PFM_GETCAB_LOW   _IOW(CSDRV_AUDIO_IOC_MAGIC, 44, int)
#define CSAUD_IOC_PFM_GETCAB_UP   _IOW(CSDRV_AUDIO_IOC_MAGIC, 45, int)
#define CSAUD_IOC_PFM_SETXPORT_CABWP    _IOW(CSDRV_AUDIO_IOC_MAGIC, 46, int)
#define CSAUD_IOC_PFM_RESET      _IOW(CSDRV_AUDIO_IOC_MAGIC, 47, int)
#define CSAUD_IOC_SET_BALANCE _IOW(CSDRV_AUDIO_IOC_MAGIC, 48, int)
#define CSAUD_IOC_GET_BALANCE _IOW(CSDRV_AUDIO_IOC_MAGIC, 49, int)
#define CSAUD_IOC_PFM_GETOFIFO_COUNT   _IOW(CSDRV_AUDIO_IOC_MAGIC, 50, int)
#define CSAUD_IOC_PFM_GETPTS_SIZE    _IOW(CSDRV_AUDIO_IOC_MAGIC, 51, int)
#define CSAUD_IOC_PFM_GETPTS_ADDR    _IOW(CSDRV_AUDIO_IOC_MAGIC, 52, int)
#define CSAUD_IOC_PFM_GETPTS_LOW    _IOW(CSDRV_AUDIO_IOC_MAGIC, 53, int)
#define CSAUD_IOC_PFM_GETPTS_UP    _IOW(CSDRV_AUDIO_IOC_MAGIC, 54, int)
#define CSAUD_IOC_PFM_GETPTS_WP    _IOW(CSDRV_AUDIO_IOC_MAGIC, 55, int)
#define CSAUD_IOC_PFM_GETPTS_RP    _IOW(CSDRV_AUDIO_IOC_MAGIC, 56, int)
#define CSAUD_IOC_PFM_SETPTS_WP    _IOW(CSDRV_AUDIO_IOC_MAGIC, 57, int)
#define CSAUD_IOC_SET_INPUTMODE    _IOW(CSDRV_AUDIO_IOC_MAGIC, 58, int)
#define CSAUD_IOC_GET_INPUTMODE    _IOW(CSDRV_AUDIO_IOC_MAGIC, 59, int)

volatile unsigned char *csdrv_audio_reg_base = NULL;
volatile unsigned char *csdrv_idcs_reg_base = NULL;
volatile unsigned char *csdrv_audio_itcm_base = NULL;
volatile unsigned char *csdrv_audio_dtcm_base = NULL;
volatile unsigned char *csdrv_audio_risc_reg_base = NULL;
volatile unsigned char *csdrv_audio_irq_reg_base = NULL;
volatile unsigned char *csdrv_audio_mix_buffer_base = NULL;
#if defined(CONFIG_ARCH_ORION_CSM1201)
volatile unsigned char *csdrv_audio_ext_code_base = NULL;
volatile unsigned char *csdrv_audio_ext_data_base = NULL;
#else
volatile unsigned char *csdrv_audio_dts_table_base = NULL;
volatile unsigned char *csdrv_audio_aac_table_base = NULL;
volatile unsigned char *csdrv_audio_eac3_table_base = NULL; /* add by ying 20081007*/
#endif
#define   REG_MAP_ADDR(x)      ((((x)>>16) == 0x4121)  ? (csdrv_audio_reg_base+((x)&0xffff)) :  \
                               ((((x)>>16) == 0x1017) ? (csdrv_idcs_reg_base+((x)&0xffff)) :      \
			       ((((x)>>16) == 0x1014) ? (csdrv_audio_irq_reg_base+((x)&0xffff)) :  \
			       ((((x)>>16) == 0x4120) ? (csdrv_audio_risc_reg_base+((x)&0xffff)) : 0 ))))

#define audio_writew(a,v)   	writew(v,REG_MAP_ADDR(a))
#define audio_writel(a,v)   	writel(v,REG_MAP_ADDR(a))
#define audio_readw(a)  	readw(REG_MAP_ADDR(a))
#define audio_readl(a)    	readl(REG_MAP_ADDR(a))

extern void xport_writel(int a,int v);
extern unsigned int xport_readl(int a);

static struct platform_device *audio_pdev;

typedef enum {
	CSDRV_AUD_DECODERTYPE = 0,
	CSDRV_AUD_BITRATE,
	CSDRV_AUD_SAMPLERATE,
	CSDRV_AUD_ACMODE,
	CSDRV_AUD_DECODEDBYTES,
	CSDRV_AUD_DECODEDFRAME,
	CSDRV_AUD_VOLUME
} CSDRV_AUD_QUERYINFO;

typedef enum {
	CSDRV_AUD_DEC_MPA = 0,
	CSDRV_AUD_DEC_AC3,
	CSDRV_AUD_DEC_AAC,
	CSDRV_AUD_DEC_DTS,
	CSDRV_AUD_DEC_LPCM,
	CSDRV_AUD_DEC_AAC_LATM,
	CSDRV_AUD_DEC_AIB
} CSDRV_AUD_DEC;

typedef enum {
	AUD_OUTPUT_I2S = 0,
	AUD_OUTPUT_SPDIFAC3,
	AUD_OUTPUT_SPDIFAAC,
	AUD_OUTPUT_SPDIFPCM,    /* not supported */
	AUD_OUTPUT_I2S_SPDIFAC3,	/* not supported */
	AUD_OUTPUT_I2S_SPDIFAAC,	/* not supported */
	AUD_OUTPUT_I2S_SPDIFPCM
} CSDRV_AUD_OUTPUTDEV;

typedef enum {
	AUD_PCM_LEFT_MONO = 0,
	AUD_PCM_RIGHT_MONO,
	AUD_PCM_STEREO,
	AUD_PCM_CHANNEL51,
	AUD_PCM_INVALID
} CSDRV_AUD_PCM_CHANNEL;

typedef enum {
	AUD_PLAYMODE_RESERVED = 0,
	AUD_PLAYMODE_PAUSE,
	AUD_PLAYMODE_RESUME,
	AUD_PLAYMODE_STOP
} CSDRV_AUD_PLAYMODE;

typedef enum {
	AUD_EQUALIZER_TYPE_DISABLE = 0,
	AUD_EQUALIZER_TYPE_POP,
	AUD_EQUALIZER_TYPE_THEATRE,
	AUD_EQUALIZER_TYPE_ROCK,
	AUD_EQUALIZER_TYPE_CLASSICAL,
	AUD_EQUALIZER_TYPE_CUSTOM = 7
} CSDRV_AUD_EQUALIZER_TYPE;

typedef struct tagAUD_EqualizerConfig {
	CSDRV_AUD_EQUALIZER_TYPE equalizer_type;
	signed char balance;
	int equalizer_band_weight[10];	/* Q27 format */
} CSDRV_AUD_EqualizerConfig;

typedef enum {
	AUD_SAMPLE_RATE_96KHZ = 0,
	AUD_SAMPLE_RATE_88_200KHZ,
	AUD_SAMPLE_RATE_64KHZ,
	AUD_SAMPLE_RATE_48KHZ,
	AUD_SAMPLE_RATE_44_100KHZ,
	AUD_SAMPLE_RATE_32KHZ,
	AUD_SAMPLE_RATE_24KHZ,
	AUD_SAMPLE_RATE_22_050KHZ,
	AUD_SAMPLE_RATE_16KHZ,
	AUD_SAMPLE_RATE_12KHZ,
	AUD_SAMPLE_RATE_11_025KHZ,
	AUD_SAMPLE_RATE_8KHZ
} CSDRV_AUD_SAMPLE_RATE;

typedef struct tagAUD_MixerConfig {
	unsigned int mixer_level;	/* Q31 format */
	CSDRV_AUD_SAMPLE_RATE mixer_sample_rate;
} CSDRV_AUD_MixerConfig;

typedef struct tagAUD_Volume {
	unsigned char front_left;
	unsigned char front_right;
	unsigned char rear_left;
	unsigned char rear_right;
	unsigned char center;
	unsigned char lfe;
} CSDRV_AUD_Volume;

typedef enum
{
	CSDRV_AUD_INPUT_NOBLOCK = 0,
	CSDRV_AUD_INPUT_BLOCK
} CSDRV_AUD_INPUT_MODE;

typedef struct {
	unsigned int ddr_rp;
	unsigned int ddr_wp;
	unsigned int rp_offset;
	unsigned int wp_offset;
	unsigned int ddr_low;
	unsigned int ddr_up;
	unsigned int rp_toggle;
	unsigned int wp_toggle;
	unsigned int size;
	unsigned int first_flag;
	unsigned int mixer_status;
	CSDRV_AUD_MixerConfig mixer_config;
} csdrv_aud_mixer;

typedef struct {
	CSDRV_AUD_Volume volume;
	CSDRV_AUD_PCM_CHANNEL channel;
	CSDRV_AUD_PLAYMODE playmode;
	CSDRV_AUD_DEC decoder_type;
	unsigned int bitrate;
	CSDRV_AUD_SAMPLE_RATE samplerate;
	CSDRV_AUD_OUTPUTDEV outputdev;
	unsigned short i2sformat;
	unsigned int acmode;
	unsigned int startdelay;
	unsigned int decodedbytes;
	unsigned int decodeframe;
	csdrv_aud_mixer mixer;
	unsigned int equalizer_status;
	CSDRV_AUD_EqualizerConfig equalizer_config;
        CSDRV_AUD_INPUT_MODE input_mode;
} csdrv_audio_information;

static csdrv_audio_information audioinfo;
static csdrv_aud_mixer *pmixer = &audioinfo.mixer;
static int errorlevel_flag = 0;
unsigned int g_audio_currnet_freq = AUD_SAMPLE_RATE_48KHZ;

DEFINE_SPINLOCK(csdrv_audio_lock);
static DECLARE_WAIT_QUEUE_HEAD(error_level_queue);
static DEFINE_SPINLOCK(error_level_lock);

const struct firmware *audio_cmd_fw = NULL;
const struct firmware *audio_data_fw= NULL;

#if !defined(CONFIG_ARCH_ORION_CSM1201)
static const struct firmware *audio_aac_part_data = NULL;
static const struct firmware *audio_aac_part_cmd  = NULL;
static const struct firmware *audio_aac_latm_data = NULL;
static const struct firmware *audio_aac_latm_cmd  = NULL;
static const struct firmware *audio_sbr_part_data = NULL;
static const struct firmware *audio_sbr_part_cmd  = NULL;
static const struct firmware *audio_ac3_part_data = NULL;
static const struct firmware *audio_ac3_part_cmd  = NULL;
static const struct firmware *audio_eac3_part_data= NULL;
static const struct firmware *audio_eac3_part_cmd = NULL;

const struct firmware *audio_dtsdata1 = NULL;
const struct firmware *audio_dtsdata2 = NULL;
const struct firmware *audio_dtsdata3 = NULL;

const struct firmware *longwin = NULL;
const struct firmware *tabsbr = NULL;
const struct firmware *taba = NULL;
const struct firmware *tabs = NULL;
const struct firmware *ntab = NULL;

const struct firmware *eac3_vq = NULL;
#endif

int __csdrv_aud_set_freq(unsigned int audio_freq);
/**********************Start of Exprort symbol*************************/
unsigned int __audio_readl(unsigned int a)
{
     	return audio_readl(a);
}

unsigned int __audio_writel(unsigned int addr ,unsigned int val)
{
	return audio_writel(addr, val);
}

EXPORT_SYMBOL(__audio_readl);
EXPORT_SYMBOL(__audio_writel);
EXPORT_SYMBOL(__csdrv_aud_set_freq);
EXPORT_SYMBOL(g_audio_currnet_freq);
/**********************End of Exprort symbol*************************/

/**********************Start of internal function***********************/
static void __csdrv_aud_set_error_level(int error_level)
{
	unsigned int reg_val = 0;
	reg_val = audio_readl(AUD_MAILBOX3);
	reg_val &= ~(0x7 << 27);
	reg_val |= (error_level << 27);
	audio_writel(AUD_MAILBOX3, reg_val);

	reg_val = audio_readl(AUD_MAILBOX2);
	reg_val |= ERROR_LEVEL_SET;
	audio_writel(AUD_MAILBOX2, reg_val);
}

static void _mixer_init(void)
{
	pmixer->first_flag = 1;
	pmixer->ddr_low = (unsigned int) csdrv_audio_mix_buffer_base;
	pmixer->ddr_up = (unsigned int) csdrv_audio_mix_buffer_base + AUD_MIX_SIZE;
	pmixer->ddr_wp = pmixer->ddr_rp = (unsigned int) csdrv_audio_mix_buffer_base;
	pmixer->wp_offset = pmixer->rp_offset = 0;
       pmixer->wp_toggle = pmixer->rp_toggle = 0;
	pmixer->size = AUD_MIX_SIZE;
        pmixer->mixer_status = 0;
}

static int __csdrv_aud_set_i2sformat(void)
{
	unsigned int reg_val = 0;

	reg_val = audio_readl(AUD_INPUT_CTRL);
	reg_val &= 0xFFE0FFFF;
	reg_val |= (audioinfo.i2sformat << 16);
	audio_writel(AUD_INPUT_CTRL, reg_val);

#ifdef DRV_AUDIO_DEBUG
	printk("i2sformat = 0x%x\n", reg_val);
#endif

	return 0;
}

static int __csdrv_aud_set_outputdev(CSDRV_AUD_OUTPUTDEV dev)
{
	unsigned int reg_val = 0;

	switch (dev) {
	case AUD_OUTPUT_I2S:
		reg_val = audio_readl(AUD_MAILBOX3);
		reg_val &= (~SPDIF_SET);
		audio_writel(AUD_MAILBOX3, reg_val);
		reg_val = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, reg_val | SPDIF_SET);

		reg_val = audio_readl(AUD_MAILBOX3);
		reg_val &= (~PCMSPDIF_SET);
		audio_writel(AUD_MAILBOX3, reg_val);
		reg_val = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, reg_val | PCMSPDIF_SET);
		break;

	case AUD_OUTPUT_SPDIFAC3:
	case AUD_OUTPUT_SPDIFAAC:
              reg_val = audio_readl(AUD_MAILBOX3);
		reg_val |= SPDIF_SET;
		audio_writel(AUD_MAILBOX3, reg_val);
		reg_val = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, reg_val | SPDIF_SET);

              reg_val = audio_readl(AUD_MAILBOX3);
		reg_val |= PCMSPDIF_SET;
		audio_writel(AUD_MAILBOX3, reg_val);
		reg_val = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, reg_val | PCMSPDIF_SET);
		break;

	case AUD_OUTPUT_SPDIFPCM:/*hardware don't support this type */
	case AUD_OUTPUT_I2S_SPDIFAC3:	/*hardware don't support this type */
	case AUD_OUTPUT_I2S_SPDIFAAC:	/*hardware don't support this type */
		break;

	case AUD_OUTPUT_I2S_SPDIFPCM:
		reg_val = audio_readl(AUD_MAILBOX3);
		reg_val |= SPDIF_SET;
		audio_writel(AUD_MAILBOX3, reg_val);
		reg_val = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, reg_val | SPDIF_SET);

		reg_val = audio_readl(AUD_MAILBOX3);
		reg_val &= (~PCMSPDIF_SET);
		audio_writel(AUD_MAILBOX3, reg_val);
		reg_val = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, reg_val | PCMSPDIF_SET);
		break;
	default:
#ifdef DRV_AUDIO_INFO
		printk("No such audio output device!\n");
#endif
		return -EINVAL;
	}
	return 0;
}

static int __csdrv_aud_get_queryinfo(CSDRV_AUD_QUERYINFO typeofinfo, void __user * pinfo)
{
	unsigned int regval = 0;
	unsigned int i = 0;

	switch (typeofinfo) {
	case CSDRV_AUD_DECODERTYPE:
#ifdef DRV_AUDIO_INFO
		printk("CSDRV_AUD_DECODERTYPE\n");
#endif
		audio_writel(AUD_MAILBOX1, AUD_ATYPE_INT);
		break;
	case CSDRV_AUD_BITRATE:
#ifdef DRV_AUDIO_INFO
		printk("CSDRV_AUD_BITRATE\n");
#endif
		audio_writel(AUD_MAILBOX1, AUD_BITRATE_INT);
		break;
	case CSDRV_AUD_SAMPLERATE:
#ifdef DRV_AUDIO_INFO
		printk("CSDRV_AUD_SAMPLERATE\n");
#endif
		audio_writel(AUD_MAILBOX1, AUD_SAMRATE_INT);
		break;
	case CSDRV_AUD_ACMODE:
#ifdef DRV_AUDIO_INFO
		printk("CSDRV_AUD_ACMODE\n");
#endif
		audio_writel(AUD_MAILBOX1, AUD_ACMOD_INT);
		break;
	case CSDRV_AUD_DECODEDBYTES:
#ifdef DRV_AUDIO_INFO
		printk("CSDRV_AUD_DECODEDBYTES\n");
#endif
		audio_writel(AUD_MAILBOX1, AUD_DECODED_BYTES_INT);
		break;
	case CSDRV_AUD_DECODEDFRAME:
#ifdef DRV_AUDIO_INFO
		printk("CSDRV_AUD_DECODEDFRAME\n");
#endif
		audio_writel(AUD_MAILBOX1, AUD_DECODED_FRAMES_INT);
		break;
	default:
#ifdef DRV_AUDIO_INFO
		printk("Don't support this query!!!%d\n", typeofinfo);
#endif
		return -EINVAL;
	}

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | REQINFO_SET);

	while ((audio_readl(AUD_MAILBOX2)) & REQINFO_SET) {
		mdelay(2);
		i++;
		if(i == 100){
			__put_user(-1, (unsigned int __user *) pinfo);	
			return 0;
		}
	}
	regval = audio_readl(AUD_MAILBOX0);
	__put_user(regval, (unsigned int __user *) pinfo);
#ifdef DRV_AUDIO_TESTINFO
	printk("regval = %d\n", regval);
	printk("regval = %d\n", *(unsigned int __user *) pinfo);
#endif
	return 0;
}

static int __csdrv_aud_set_equalizer_status(unsigned int is_equalizer)
{
	unsigned int regval = 0;

	if (is_equalizer == 0) {	/*disable equalizer and set channel balance to 0 */
		regval = audio_readl(AUD_MAILBOX3);
		regval = regval & 0xf8ffffff;	//fixme
		audio_writel(AUD_MAILBOX3, regval);
		regval = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, regval | EQUALIZE_SET);

		audioinfo.equalizer_config.balance = 0;
		audioinfo.equalizer_status = 0;

		regval = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, regval | CHANNEL_BALANCE);
	}
	else {			/*enable equalizer */
		/*first ,firmware need disable equalizer */
		regval = audio_readl(AUD_MAILBOX3);
		regval = regval & 0xf8ffffff;	//fixme
		audio_writel(AUD_MAILBOX3, regval);
		regval = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, regval | EQUALIZE_SET);
		/*disable end */

		regval = audio_readl(AUD_MAILBOX3);
		regval = regval | ((audioinfo.equalizer_config.equalizer_type) << 24);
		audio_writel(AUD_MAILBOX3, regval);

		regval = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, regval | EQUALIZE_SET);
	}
	return 0;
}

#if 0
static int __csdrv_aud_set_equalizer_config(CSDRV_AUD_EqualizerConfig * equalizer_config)
{
	unsigned int regval = 0;

	/*first ,firmware need disable equalizer */
	regval = audio_readl(AUD_MAILBOX3);
	regval = regval & 0xf8ffffff;	//fixme
	audio_writel(AUD_MAILBOX3, regval);
	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | EQUALIZE_SET);
	/*disable end */

	regval = audio_readl(AUD_MAILBOX3);

	regval = regval | ((equalizer_config->equalizer_type) << 24);
	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | EQUALIZE_SET);
	return 0;
}
#endif
static int __csdrv_aud_set_decodedbytes(unsigned int decoded_bytes)
{
	unsigned int regval = 0;

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | DECODEDBYTE_SET);
	udelay(10);
	return 0;
}

static int __csdrv_aud_set_startdelay(unsigned int ms_second)
{
	unsigned int regval = 0;

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | STARTDELAY_SET);
	udelay(10);
	return 0;
}

static int __csdrv_aud_set_playmode(CSDRV_AUD_PLAYMODE playmode)
{
	unsigned int regval = 0;
    
	regval = audio_readl(AUD_MAILBOX3);
	switch (playmode) {	/*fixme */
	case AUD_PLAYMODE_PAUSE:
		regval &= 0xFFFE7FFF;
		regval |= 0x8000;
		break;
	case AUD_PLAYMODE_RESUME:
		regval &= 0xFFFE7FFF;
		regval |= 0x10000;
		break;
	case AUD_PLAYMODE_STOP:
		regval &= 0xFFFE7FFF;
		regval |= 0x18000;
		break;
	default:
		break;
	}
	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | PLAYMODE_SET);

	return 0;
}

static int __csdrv_aud_set_output_channel(CSDRV_AUD_PCM_CHANNEL channel_mode)
{
	unsigned int regval = 0;

	regval = audio_readl(AUD_MAILBOX3);
	switch (channel_mode) {
	case AUD_PCM_LEFT_MONO:
		regval &= 0xFFFF87FF;
		break;
	case AUD_PCM_RIGHT_MONO:
		regval &= 0xFFFF87FF;
		regval |= 0x800;
		break;
	case AUD_PCM_STEREO:
		regval &= 0xFFFF87FF;
		regval |= 0x1000;
		break;
	case AUD_PCM_CHANNEL51:
		regval &= 0xFFFF87FF;
		regval |= 0x1800;
		break;
	default:
#ifdef DRV_AUDIO_INFO
		printk("No such output channel type!\n");
#endif
		return -EINVAL;
	}
	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | OUTCH_SET);
	return 0;
}

static int __csdrv_aud_set_mixer_status(unsigned int is_mixer)
{
	unsigned int regval = 0;
	//disable mixer,not support enable
	regval = audio_readl(AUD_MAILBOX3);
	if (is_mixer == 0)
		regval = regval & (~MIX_SET);
	else
		return 0;
	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | MIX_SET);
	return 0;
}

static int __csdrv_aud_set_mixer_config(CSDRV_AUD_MixerConfig * mixer_config)
{
	unsigned int regval = 0;
        
    //enable mixer
        regval = audio_readl(AUD_MAILBOX3);
	regval = regval | MIX_SET;
	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | MIX_SET);

	return 0;
}

static int __csdrv_aud_write_mixer_buffer(char *src, int size)
{
	unsigned int tempsize = 0;
	unsigned long flags;
#ifdef DRV_AUDIO_TESTINFO
	printk("size = %d\n", size);
	printk("(1)\n");
#endif
	while (size&&audioinfo.mixer.mixer_status) {
#ifdef DRV_AUDIO_TESTINFO
		printk("(2)\n");
#endif

		spin_lock_irqsave(&csdrv_audio_lock, flags);
		if (pmixer->first_flag)	//first write
		{
#ifdef DRV_AUDIO_TESTINFO
			printk("(3)\n");
#endif
			if (size >= AUD_MIX_SIZE) {
#ifdef DRV_AUDIO_TESTINFO
				printk("(3-1)\n");
#endif
				__copy_from_user((char *) pmixer->ddr_wp, (char __user *) src, AUD_MIX_SIZE);
				pmixer->ddr_wp = pmixer->ddr_low;
				pmixer->wp_offset = 0;
				pmixer->wp_toggle = 1;
				size -= AUD_MIX_SIZE;
				src += AUD_MIX_SIZE;
			}
			else {
#ifdef DRV_AUDIO_TESTINFO
				printk("(3-2)\n");
#endif
				__copy_from_user((char *) pmixer->ddr_wp, (char __user *) src, size);
				pmixer->ddr_wp += size;
				pmixer->wp_offset += size;
				size = 0;
			}
#ifdef DRV_AUDIO_TESTINFO
			printk("(6)\n");
#endif
			pmixer->first_flag = 0;
		}
		else if (pmixer->first_flag == 0)	//not first
		{
#ifdef DRV_AUDIO_TESTINFO
			printk("(7)\n");
#endif
			if (pmixer->wp_toggle == pmixer->rp_toggle)	//no wrap
			{
#ifdef DRV_AUDIO_TESTINFO
				printk("(8)\n");
#endif
				if (pmixer->ddr_wp >= pmixer->ddr_rp)	//host can write data to mixer
				{
#ifdef DRV_AUDIO_TESTINFO
					printk("(9)\n");
#endif
					tempsize = pmixer->ddr_up - pmixer->ddr_wp;
					if (size < tempsize)	//mix buffer have enough space
					{
#ifdef DRV_AUDIO_TESTINFO
						printk("(9-1)\n");
#endif
						__copy_from_user((char *) pmixer->ddr_wp, (char __user *) src, size);
						pmixer->ddr_wp += size;
						pmixer->wp_offset += size;
						size = 0;
					}
					else if (size >= tempsize)	//mix buffer have not enough space, write pointer wrap!
					{
#ifdef DRV_AUDIO_TESTINFO
						printk("(9-2)\n");
#endif
						__copy_from_user((char *) pmixer->ddr_wp, (char __user *) src,
								 tempsize);
						pmixer->ddr_wp = pmixer->ddr_low;
						pmixer->wp_offset = 0;
						if (pmixer->wp_toggle)
							pmixer->wp_toggle = 0;
						else
							pmixer->wp_toggle = 1;
						size -= tempsize;
						src += tempsize;
					}
				}
				else if (pmixer->ddr_wp < pmixer->ddr_rp)	//host can't write data to mixer
				{
#ifdef DRV_AUDIO_TESTINFO
					printk("(10)\n");
#endif
					//pmixer->ddr_wp = pmixer->ddr_rp;
					//pmixer->wp_offset = pmixer->rp_offset;
					udelay(3);
					//continue;
				}
			}
			else if (pmixer->wp_toggle != pmixer->rp_toggle)	//wrap
			{
#ifdef DRV_AUDIO_TESTINFO
				printk("(11)\n");
#endif
				if (pmixer->ddr_wp < pmixer->ddr_rp)	//host can write data to mixer
				{
#ifdef DRV_AUDIO_TESTINFO
					printk("(12)\n");
#endif
					tempsize = pmixer->ddr_rp - pmixer->ddr_wp;
					if (size < tempsize)	//mix buffer have enough space
					{
#ifdef DRV_AUDIO_TESTINFO
						printk("(12-1)\n");
#endif
						__copy_from_user((char *) pmixer->ddr_wp, (char __user *) src, size);
						pmixer->ddr_wp += size;
						pmixer->wp_offset += size;
						size = 0;
					}
					else if (size >= tempsize)	//mix buffer have not enough space, write residual size!
					{
#ifdef DRV_AUDIO_TESTINFO
						printk("(12-2)\n");
#endif
						__copy_from_user((char *) pmixer->ddr_wp, (char __user *) src,
								 tempsize);
						pmixer->ddr_wp = pmixer->ddr_rp;
						pmixer->wp_offset += tempsize;
						size -= tempsize;
						src += tempsize;
					}
				}
				else if (pmixer->ddr_wp >= pmixer->ddr_rp)	//host can't write data to mixer
				{
#ifdef DRV_AUDIO_TESTINFO
                                        if(pmixer->ddr_wp == pmixer->ddr_rp)
					    printk("(13-1)\n");
                                        else if(pmixer->ddr_wp > pmixer->ddr_rp)
                                            printk("(13-2)\n");
#endif
					udelay(3);
					//continue;
				}
			}
		}
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		schedule();
#ifdef DRV_AUDIO_TESTINFO
		printk("(14)\n");
#endif
	}
#ifdef DRV_AUDIO_TESTINFO
	printk("15\n");
	udelay(1000);
		
#endif
	return 0;
}

/* load a table to ram */
static int __load_table_to_ram(volatile unsigned char * pDst, unsigned char * pSrc, unsigned int len_in_byte)
{
        int i = 0;

        for (i = 0; i < len_in_byte; i++)
                *pDst++ = *pSrc++;

        return 0;
}
#if !defined(CONFIG_ARCH_ORION_CSM1201)
static int __load_dts_table(void)
{
	int ret = 0;

	ret = request_firmware(&audio_dtsdata1, "audio_dts_adpcm_vb.bin", &(audio_pdev->dev));
	if (ret != 0 || audio_dtsdata1 == NULL ) {
		printk(KERN_ERR "Failed to load DTS adpcm firmware data section\n");
		return -1;
	}
	ret = request_firmware(&audio_dtsdata2, "audio_dts_nonpd.bin", &(audio_pdev->dev));
	if (ret != 0 || audio_dtsdata2 == NULL ) {
		printk(KERN_ERR "Failed to load DTS nonpd firmware data section\n");
        release_firmware(audio_dtsdata1);
        audio_dtsdata1=NULL;
		return -1;
	}
	ret = request_firmware(&audio_dtsdata3, "audio_dts_pd.bin", &(audio_pdev->dev));
	if (ret != 0 || audio_dtsdata3 == NULL ) {
		printk(KERN_ERR "Failed to load DTS pd firmware data section\n");
        release_firmware(audio_dtsdata1);
        audio_dtsdata1=NULL;
        release_firmware(audio_dtsdata2);
        audio_dtsdata2=NULL;
		return -1;
	}
    audio_writel(AUD_MAILBOX4, 0xbeef0000 | (AUD_DTS_TABLE_REGION & 0x0000ffff));
    audio_writel(AUD_MAILBOX5, 0x0000cafe |(AUD_DTS_TABLE_REGION & 0xffff0000));
    
    __load_table_to_ram((unsigned char * )csdrv_audio_dts_table_base, (unsigned char *)audio_dtsdata1->data, 32768);
    __load_table_to_ram((unsigned char * )(csdrv_audio_dts_table_base + 32768), (unsigned char *)audio_dtsdata2->data, 2048);
    __load_table_to_ram((unsigned char * )(csdrv_audio_dts_table_base + 32768 + 2048), (unsigned char *)audio_dtsdata3->data, 2048);
    
    release_firmware(audio_dtsdata1); audio_dtsdata1 = NULL;
    release_firmware(audio_dtsdata2); audio_dtsdata2 = NULL;
    release_firmware(audio_dtsdata3); audio_dtsdata3 = NULL;
    return 0;
}

// AAC const data in ddr
#define AAC_KBD_OFFSET					0x0      //(AAC_RIGHTCH_OFFSET + AAC_RIGHTCH_SIZE)	// 0xa100
#define AAC_KBD_SIZE					0x1000   // 4kB, 1024 int??
// SBR const data in ddr
#define SBR_HUFFMAN_TAB_OFFSET			(AAC_KBD_OFFSET+AAC_KBD_SIZE)
#define SBR_HUFFMAN_TAB_SIZE			0x0600	// 1536 (1208)

#define SBR_QMFA_WIN_OFFSET				(SBR_HUFFMAN_TAB_OFFSET+SBR_HUFFMAN_TAB_SIZE)
#define SBR_QMFA_WIN_SIZE				0x0300	// 768 (660)

#define SBR_QMFS_WIN_OFFSET				(SBR_QMFA_WIN_OFFSET+SBR_QMFA_WIN_SIZE)
#define SBR_QMFS_WIN_SIZE				0x0a00	// 2560

#define SBR_NOISE_TAB_OFFSET			(SBR_QMFS_WIN_OFFSET+SBR_QMFS_WIN_SIZE)
#define SBR_NOISE_TAB_SIZE				0x1000	// 4096

#define HEAAC_RWDATA_OFFSET				(SBR_NOISE_TAB_OFFSET + SBR_NOISE_TAB_SIZE)
#define HEAAC_RWDATA_SIZE		(AUD_HEAAC_TABLE_SIZE - AAC_KBD_SIZE -SBR_HUFFMAN_TAB_SIZE -SBR_QMFA_WIN_SIZE -SBR_QMFS_WIN_SIZE - SBR_NOISE_TAB_SIZE)
static int __load_aac_table(void)
{
	int ret;

	ret = request_firmware(&longwin, "audio_aac_longwindow.bin", &(audio_pdev->dev));
	if (ret != 0 || longwin == NULL ) {
		printk(KERN_ERR "Failed to load AAC longwin section\n");
		return -1;
	}
	ret = request_firmware(&tabsbr, "audio_aac_tabsbr.bin", &(audio_pdev->dev));
	if (ret != 0 || tabsbr == NULL ) {
		printk(KERN_ERR "Failed to load AAC tabsbr section\n");
        release_firmware(longwin);
        longwin=NULL;
		return -1;
	}
	ret = request_firmware(&taba, "audio_aac_taba.bin", &(audio_pdev->dev));
	if (ret != 0 || taba == NULL ) {
		printk(KERN_ERR "Failed to load AAC taba section\n");
        release_firmware(longwin);
        longwin=NULL;
        release_firmware(tabsbr);
        longwin=NULL;
		return -1;
	}
	ret = request_firmware(&tabs, "audio_aac_tabs.bin", &(audio_pdev->dev));
	if (ret != 0 || tabs == NULL ) {
		printk(KERN_ERR "Failed to load AAC tabs section\n");
        release_firmware(longwin);
        longwin=NULL;
        release_firmware(tabsbr);
        longwin=NULL;
        release_firmware(taba);
        taba=NULL;

		return -1;
	}
	ret = request_firmware(&ntab, "audio_aac_ntab.bin", &(audio_pdev->dev));
	if (ret != 0 || ntab == NULL ) {
		printk(KERN_ERR "Failed to load AAC ntab section\n");
        release_firmware(longwin);
        longwin=NULL;
        release_firmware(tabsbr);
        longwin=NULL;
        release_firmware(taba);
        taba=NULL;
        release_firmware(tabs);
        tabs=NULL;
		return -1;
	}
	
       __load_table_to_ram(
	   	(unsigned char * )csdrv_audio_aac_table_base  + AAC_KBD_OFFSET, (unsigned char *)longwin->data, longwin->size);
       __load_table_to_ram(
	   	(unsigned char * )(csdrv_audio_aac_table_base + SBR_HUFFMAN_TAB_OFFSET), (unsigned char *)tabsbr->data, tabsbr->size);
       __load_table_to_ram(
	   	(unsigned char * )(csdrv_audio_aac_table_base + SBR_QMFA_WIN_OFFSET), (unsigned char *)taba->data, taba->size);
       __load_table_to_ram(
	   	(unsigned char * )(csdrv_audio_aac_table_base + SBR_QMFS_WIN_OFFSET), (unsigned char *)tabs->data, tabs->size);
	   __load_table_to_ram(
	   	(unsigned char * )(csdrv_audio_aac_table_base + SBR_NOISE_TAB_OFFSET), (unsigned char *)ntab->data, ntab->size);

		memset((unsigned char * )csdrv_audio_aac_table_base + HEAAC_RWDATA_OFFSET,
                                                           0, HEAAC_RWDATA_SIZE);
		printk("clear 0x%08x +0x%08x bytes of he aac rw data\n", HEAAC_RWDATA_OFFSET, HEAAC_RWDATA_SIZE);
	
	release_firmware(longwin);
	release_firmware(tabsbr);
	release_firmware(taba);
	release_firmware(tabs);
	release_firmware(ntab);
	
	return 0;
}

static int __load_eac3_table(void)
{
	int ret;


	ret = request_firmware(&eac3_vq, "audio_eac3_vq.bin", &(audio_pdev->dev));
	if (ret != 0 || eac3_vq == NULL ) {
		printk(KERN_ERR "Failed to load AC3 VQ section\n");
		return -1;
	}
    __load_table_to_ram(
                        (unsigned char * )csdrv_audio_eac3_table_base, (unsigned char *)eac3_vq->data, eac3_vq->size);
    
    printk("load eac3 table to ddr!\n");  
	
    release_firmware(eac3_vq);
    
    return 0;
}
#endif  
static void __load_audio_firmware(volatile unsigned char *pDst, unsigned char *pSrc, int len_in_byte)
{
#if defined(CONFIG_ARCH_ORION_CSM1201)	
	unsigned long tmp_val;
	volatile unsigned long *tcm_addr = (volatile unsigned long*)pDst;
	int tcm_wr_size = len_in_byte>>2;
		
	while(tcm_wr_size-- > 0) {
		tmp_val  = (unsigned long)(*pSrc++);
		tmp_val |= (unsigned long)(*pSrc++) << 8;
		tmp_val |= (unsigned long)(*pSrc++) << 16;
		tmp_val |= (unsigned long)(*pSrc++) << 24;
			
		*tcm_addr++ = tmp_val;
	}
#else
	memcpy((unsigned char *)pDst, pSrc, len_in_byte);	
#endif

	return;
}


#if !defined(CONFIG_ARCH_ORION_CSM1201)

static int __csdrv_load_sbr_extra_firmware(void)
{
	int ret;
	if (NULL == audio_sbr_part_data) {
		ret = request_firmware(&audio_sbr_part_data, "audio_sbrpart_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_sbr_part_data == NULL ) {
			printk(KERN_ERR "Failed to load AAC/AACLATM SBRPART firmware data section\n");
            return -1;
		}
	}
	if (NULL == audio_sbr_part_cmd) {
		ret = request_firmware(&audio_sbr_part_cmd, "audio_sbrpart_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_sbr_part_cmd == NULL ) {
			printk(KERN_ERR "Failed to load AAC/AACLATM SBRPART firmware CMD section\n");
            release_firmware(audio_sbr_part_data);
            audio_sbr_part_data = NULL;
            return -1;
		}
	}
   
	return 0;
}

static int __csdrv_load_aac_extra_part_firmware(void)
{
	int ret;
	if (NULL == audio_aac_part_data) {
		ret = request_firmware(&audio_aac_part_data, "audio_aacpart_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_aac_part_data == NULL ) {
			printk(KERN_ERR "Failed to load AAC firmware data section\n");
            return -1;
		}
	}
	if (NULL == audio_aac_part_cmd) {
		ret = request_firmware(&audio_aac_part_cmd, "audio_aacpart_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_aac_part_cmd == NULL ) {
			printk(KERN_ERR "Failed to load AAC firmware CMD section\n");
            release_firmware(audio_aac_part_data);
            audio_aac_part_data = NULL;
            return -1;
		}
	}
    if(__csdrv_load_sbr_extra_firmware() !=0) {
        release_firmware(audio_aac_part_data);
        audio_aac_part_data = NULL;
        release_firmware(audio_aac_part_cmd);
        audio_aac_part_cmd = NULL;
        return -1;
    }
    return 0;
}
static int __csdrv_load_aac_extra_latm_firmware(void)
{
	int ret;
	if (NULL == audio_aac_latm_data) {
		ret = request_firmware(&audio_aac_latm_data, "audio_aaclatm_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_aac_latm_data == NULL ) {
			printk(KERN_ERR "Failed to load AACLATM firmware data section\n");
            return -1;
        }
	}
	if (NULL == audio_aac_latm_cmd) {
		ret = request_firmware(&audio_aac_latm_cmd, "audio_aaclatm_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_aac_latm_cmd == NULL ) {
			printk(KERN_ERR "Failed to load AACLATM firmware CMD section\n");
            release_firmware(audio_aac_latm_data);
            audio_aac_latm_data = NULL;
            return -1;
		}
	}
    if(__csdrv_load_sbr_extra_firmware() !=0) {
        release_firmware(audio_aac_latm_data);
        audio_aac_latm_data = NULL;
        release_firmware(audio_aac_latm_cmd);
        audio_aac_latm_cmd = NULL;
        return -1;
    }
    return 0;
}


static int __csdrv_load_ac3_extra_firmware(void)
{
	int ret;
	if (NULL == audio_ac3_part_data) {
		ret = request_firmware(&audio_ac3_part_data, "audio_ac3_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_ac3_part_data == NULL ) {
			printk(KERN_ERR "Failed to load AC3 firmware data section\n");
            return -1;
		}
	}
	if (NULL == audio_ac3_part_cmd) {
		ret = request_firmware(&audio_ac3_part_cmd, "audio_ac3_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_ac3_part_cmd == NULL ) {
			printk(KERN_ERR "Failed to load AC3 firmware CMD section\n");
            release_firmware(audio_ac3_part_data);
            audio_ac3_part_data = NULL;
            return -1;
        }
	}
    return 0;
}


static int __csdrv_load_eac3_extra_firmware(void)
{
    int ret;
	if (NULL == audio_eac3_part_data) {
		ret = request_firmware(&audio_eac3_part_data, "audio_eac3_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_eac3_part_data == NULL ) {
			printk(KERN_ERR "Failed to load EAC3 firmware part data section\n");
            return -1;
		}
	}
	if (NULL == audio_eac3_part_cmd) {
		ret = request_firmware(&audio_eac3_part_cmd, "audio_eac3_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_eac3_part_cmd == NULL ) {
			printk(KERN_ERR "Failed to load EAC3 firmware part CMD section\n");
            release_firmware(audio_eac3_part_data);
            audio_eac3_part_data = NULL;
            return -1;
		}
	}
    return 0;
}

static int __csdrv_aud_set_aac_part(void)
{
	unsigned int itcm_wr_size, dtcm_wr_size;
	unsigned char *itcm_wr_src = NULL;
	unsigned char *dtcm_wr_src = NULL;
	volatile unsigned char *itcm_wr_dest = csdrv_audio_itcm_base;
	volatile unsigned char *dtcm_wr_dest = csdrv_audio_dtcm_base;
    if ((audio_aac_part_cmd == NULL) || (audio_aac_part_data == NULL))
        return -1;

	itcm_wr_size = audio_aac_part_cmd->size;
	dtcm_wr_size = audio_aac_part_data->size;
	itcm_wr_src = audio_aac_part_cmd->data;
	dtcm_wr_src = audio_aac_part_data->data;

	audio_writel(AUD_RISC_CTRL, 1 << 25);

	audio_writel(AUD_MAILBOX5, AUD_HEAAC_TABLE_REGION);

	if (itcm_wr_size > CSDRV_AUD_ITCM_SIZE)
		itcm_wr_size = CSDRV_AUD_ITCM_SIZE;
	if (dtcm_wr_size > CSDRV_AUD_DTCM_SIZE)
		dtcm_wr_size = CSDRV_AUD_DTCM_SIZE;

	__load_audio_firmware((unsigned char *)itcm_wr_dest, itcm_wr_src, itcm_wr_size);
	__load_audio_firmware((unsigned char *)dtcm_wr_dest, dtcm_wr_src, dtcm_wr_size);

	audio_writel(AUD_RISC_CTRL, 0x0); // start mips

	return 0;
}

static int __csdrv_aud_set_aac_latm(void)
{
	unsigned int itcm_wr_size, dtcm_wr_size;
	unsigned char *itcm_wr_src = NULL;
	unsigned char *dtcm_wr_src = NULL;
	volatile unsigned char *itcm_wr_dest = csdrv_audio_itcm_base;
	volatile unsigned char *dtcm_wr_dest = csdrv_audio_dtcm_base;
    if ((audio_aac_latm_cmd == NULL) || (audio_aac_latm_data == NULL))
        return -1;

	
	itcm_wr_size = audio_aac_latm_cmd->size;
	dtcm_wr_size = audio_aac_latm_data->size;
	itcm_wr_src = audio_aac_latm_cmd->data;
	dtcm_wr_src = audio_aac_latm_data->data;

	audio_writel(AUD_RISC_CTRL, 1 << 25);

	audio_writel(AUD_MAILBOX5, AUD_HEAAC_TABLE_REGION);

	if (itcm_wr_size > CSDRV_AUD_ITCM_SIZE)
		itcm_wr_size = CSDRV_AUD_ITCM_SIZE;
	if (dtcm_wr_size > CSDRV_AUD_DTCM_SIZE)
		dtcm_wr_size = CSDRV_AUD_DTCM_SIZE;

	__load_audio_firmware((unsigned char *)itcm_wr_dest, itcm_wr_src, itcm_wr_size);
	__load_audio_firmware((unsigned char *)dtcm_wr_dest, dtcm_wr_src, dtcm_wr_size);

	audio_writel(AUD_RISC_CTRL, 0x0); // start mips

	return 0;
}

static int __csdrv_aud_set_sbr_part(void)
{
	unsigned int itcm_wr_size, dtcm_wr_size;
	unsigned char *itcm_wr_src = NULL;
	unsigned char *dtcm_wr_src = NULL;
	volatile unsigned char *itcm_wr_dest = csdrv_audio_itcm_base;
	volatile unsigned char *dtcm_wr_dest = csdrv_audio_dtcm_base;

    if ((audio_sbr_part_cmd == NULL) || (audio_sbr_part_data == NULL))
        return -1;

	itcm_wr_size = audio_sbr_part_cmd->size;
	dtcm_wr_size = audio_sbr_part_data->size;
	itcm_wr_src = audio_sbr_part_cmd->data;
	dtcm_wr_src = audio_sbr_part_data->data;

	audio_writel(AUD_RISC_CTRL, 1 << 25);

	audio_writel(AUD_MAILBOX5, AUD_HEAAC_TABLE_REGION);

	if (itcm_wr_size > CSDRV_AUD_ITCM_SIZE)
		itcm_wr_size = CSDRV_AUD_ITCM_SIZE;
	if (dtcm_wr_size > CSDRV_AUD_DTCM_SIZE)
		dtcm_wr_size = CSDRV_AUD_DTCM_SIZE;

	__load_audio_firmware((unsigned char *)itcm_wr_dest, itcm_wr_src, itcm_wr_size);
	__load_audio_firmware((unsigned char *)dtcm_wr_dest, dtcm_wr_src, dtcm_wr_size);
	
	audio_writel(AUD_RISC_CTRL, 0x0); // start mips

	return 0;
}

static int __csdrv_aud_set_ac3_part(void)
{
	unsigned int itcm_wr_size, dtcm_wr_size;
	unsigned char *itcm_wr_src = NULL;
	unsigned char *dtcm_wr_src = NULL;
	volatile unsigned char *itcm_wr_dest = csdrv_audio_itcm_base;
	volatile unsigned char *dtcm_wr_dest = csdrv_audio_dtcm_base;
	
    if ((audio_ac3_part_cmd == NULL) || (audio_ac3_part_data == NULL))
        return -1;

	itcm_wr_size = audio_ac3_part_cmd->size;
	dtcm_wr_size = audio_ac3_part_data->size;
	itcm_wr_src = audio_ac3_part_cmd->data;
	dtcm_wr_src = audio_ac3_part_data->data;

	audio_writel(AUD_RISC_CTRL, 1 << 25);

	audio_writel(AUD_MAILBOX5, AUD_EAC3_TABLE_REGION);

	if (itcm_wr_size > CSDRV_AUD_ITCM_SIZE)
		itcm_wr_size = CSDRV_AUD_ITCM_SIZE;
	if (dtcm_wr_size > CSDRV_AUD_DTCM_SIZE)
		dtcm_wr_size = CSDRV_AUD_DTCM_SIZE;
#if 0
    tmp[0] = audio_readl(AUD_STC);
    tmp[1] = audio_readl(AUD_STC);
    STC2 = (tmp[0] >> 1) | (tmp[1] << 31);
#endif
	__load_audio_firmware((unsigned char *)itcm_wr_dest, itcm_wr_src, itcm_wr_size);
	__load_audio_firmware((unsigned char *)dtcm_wr_dest, dtcm_wr_src, dtcm_wr_size);

#if 0
    tmp[0] = audio_readl(AUD_STC);
    tmp[1] = audio_readl(AUD_STC);
    STC3 = (tmp[0] >> 1) | (tmp[1] << 31);

    printk("STC1 = %x\n", STC1);
    printk("STC2 = %x\n", STC2);
    printk("STC3 = %x\n", STC3);
#endif
	audio_writel(AUD_RISC_CTRL, 0x0); // start mips

	return 0;
}

static int __csdrv_aud_set_eac3_part(void)
{
	unsigned int itcm_wr_size, dtcm_wr_size;
	unsigned char *itcm_wr_src = NULL;
	unsigned char *dtcm_wr_src = NULL;
	volatile unsigned char *itcm_wr_dest = csdrv_audio_itcm_base;
	volatile unsigned char *dtcm_wr_dest = csdrv_audio_dtcm_base;

    if ((audio_eac3_part_cmd == NULL) || (audio_eac3_part_data == NULL))
        return -1;
	
	itcm_wr_size = audio_eac3_part_cmd->size;
	dtcm_wr_size = audio_eac3_part_data->size;
	itcm_wr_src = audio_eac3_part_cmd->data;
	dtcm_wr_src = audio_eac3_part_data->data;

	audio_writel(AUD_RISC_CTRL, 1 << 25);

	audio_writel(AUD_MAILBOX5, AUD_EAC3_TABLE_REGION);

	if (itcm_wr_size > CSDRV_AUD_ITCM_SIZE)
		itcm_wr_size = CSDRV_AUD_ITCM_SIZE;
	if (dtcm_wr_size > CSDRV_AUD_DTCM_SIZE)
		dtcm_wr_size = CSDRV_AUD_DTCM_SIZE;

	__load_audio_firmware((unsigned char *)itcm_wr_dest, itcm_wr_src, itcm_wr_size);
	__load_audio_firmware((unsigned char *)dtcm_wr_dest, dtcm_wr_src, dtcm_wr_size);

	audio_writel(AUD_RISC_CTRL, 0x0); // start mips

	return 0;
}
#endif


static int __csdrv_aud_set_dec(CSDRV_AUD_DEC dec_type)
{

	
	int ret;
	unsigned int itcm_wr_size, dtcm_wr_size;
	unsigned char *itcm_wr_src = NULL;
	unsigned char *dtcm_wr_src = NULL;
	volatile unsigned char *itcm_wr_dest = csdrv_audio_itcm_base;
	volatile unsigned char *dtcm_wr_dest = csdrv_audio_dtcm_base;

    if (audio_cmd_fw != NULL){
        release_firmware(audio_cmd_fw);
    }

    if (audio_data_fw != NULL){
        release_firmware(audio_cmd_fw);
    }

	switch (dec_type) {
		
	case CSDRV_AUD_DEC_LPCM:
		ret = request_firmware(&audio_data_fw, "audio_lpcm_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load LPCM firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_lpcm_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load LPCM firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw=NULL;
			return -1;
		}
		itcm_wr_size = audio_cmd_fw->size;
		dtcm_wr_size = audio_data_fw->size;
		itcm_wr_src = audio_cmd_fw->data;
		dtcm_wr_src = audio_data_fw->data;
		break;
	
	case CSDRV_AUD_DEC_DTS:
		ret = request_firmware(&audio_data_fw, "audio_dts_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load DTS firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_dts_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load DTS firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
		itcm_wr_size = audio_cmd_fw->size;
		dtcm_wr_size = audio_data_fw->size;
		itcm_wr_src = audio_cmd_fw->data;
		dtcm_wr_src = audio_data_fw->data;
		break;
	/*-------------------------------------------------*/
	
	case CSDRV_AUD_DEC_AAC:
#if !defined(CONFIG_ARCH_ORION_CSM1201)		
		ret = request_firmware(&audio_data_fw, "audio_aacpart_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load DTS firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_aacpart_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load DTS firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
		itcm_wr_size = audio_cmd_fw->size;
		dtcm_wr_size = audio_data_fw->size;
		itcm_wr_src = audio_cmd_fw->data;
		dtcm_wr_src = audio_data_fw->data;
#else
		ret = request_firmware(&audio_data_fw, "audio_aacadts_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load DTS firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_aacadts_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load DTS firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
		itcm_wr_size = audio_cmd_fw->size;
		dtcm_wr_size = audio_data_fw->size;
		itcm_wr_src = audio_cmd_fw->data;
		dtcm_wr_src = audio_data_fw->data;
#endif
		break;

	case CSDRV_AUD_DEC_AAC_LATM:
#if !defined(CONFIG_ARCH_ORION_CSM1201)			
		ret = request_firmware(&audio_data_fw, "audio_aaclatm_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load AACLATM firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_aaclatm_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load AACLATM firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
		itcm_wr_size = audio_cmd_fw->size;
		dtcm_wr_size = audio_data_fw->size;
		itcm_wr_src = audio_cmd_fw->data;
		dtcm_wr_src = audio_data_fw->data;
#else
		ret = request_firmware(&audio_data_fw, "audio_aaclatm_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load AACLATM firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_aaclatmddr_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load AACLATM firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
		itcm_wr_size = audio_cmd_fw->size;
		dtcm_wr_size = audio_data_fw->size;
		itcm_wr_src = audio_cmd_fw->data;
		dtcm_wr_src = audio_data_fw->data;
#endif
		break;
		
	case CSDRV_AUD_DEC_AC3:
#if defined(CONFIG_ARCH_ORION_CSM1201) //#ifdef SUPPORT_EAC3_1201
        ret = request_firmware(&audio_data_fw, "audio_eac3_data.bin", &(audio_pdev->dev));
        if (ret != 0 || audio_data_fw == NULL ) {
            printk(KERN_ERR "Failed to load EAC3 asdfirmware data section\n");
            return -1;
        }
        ret = request_firmware(&audio_cmd_fw, "audio_eac3_cmd.bin", &(audio_pdev->dev));
        if (ret != 0 || audio_cmd_fw == NULL ) {
            printk(KERN_ERR "Failed to load EAC3 firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
            return -1;
        }
#else
		ret = request_firmware(&audio_data_fw, "audio_ac3_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load AC3 firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_ac3_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load AC3 firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
#endif        
		itcm_wr_size = audio_cmd_fw->size;
		dtcm_wr_size = audio_data_fw->size;
		itcm_wr_src = audio_cmd_fw->data;
		dtcm_wr_src = audio_data_fw->data;
		break;

#if defined(CONFIG_ARCH_ORION_CSM1201)
	case CSDRV_AUD_DEC_AIB:
		ret = request_firmware(&audio_data_fw, "audio_aib_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load AC3 firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_aib_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load AC3 firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
		itcm_wr_size = audio_cmd_fw->size;
		dtcm_wr_size = audio_data_fw->size;
		itcm_wr_src = audio_cmd_fw->data;
		dtcm_wr_src = audio_data_fw->data;
		break;
#endif
	default:
		ret = request_firmware(&audio_data_fw, "audio_mp2_data.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load MP2 firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_mp2_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load MP2 firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
		itcm_wr_size = audio_cmd_fw->size;
		dtcm_wr_size = audio_data_fw->size;
		itcm_wr_src = audio_cmd_fw->data;
		dtcm_wr_src = audio_data_fw->data;
	}
	audio_writel(AUD_RISC_CTRL, 1 << 25);

	if (itcm_wr_size > CSDRV_AUD_ITCM_SIZE)
		itcm_wr_size = CSDRV_AUD_ITCM_SIZE;
	if (dtcm_wr_size > CSDRV_AUD_DTCM_SIZE)
		dtcm_wr_size = CSDRV_AUD_DTCM_SIZE;

	__load_audio_firmware(itcm_wr_dest, itcm_wr_src, itcm_wr_size);
	__load_audio_firmware(dtcm_wr_dest, dtcm_wr_src, dtcm_wr_size);

    release_firmware(audio_cmd_fw); audio_cmd_fw = NULL;
    release_firmware(audio_data_fw);audio_data_fw= NULL;

#if !defined(CONFIG_ARCH_ORION_CSM1201)
	/* if dts, load ADPCM, NONE_PERFECT_QMF, PERFECT_QMF*/
	if(CSDRV_AUD_DEC_DTS == dec_type){
		ret = __load_dts_table();	
    }
	if(CSDRV_AUD_DEC_AAC== dec_type || CSDRV_AUD_DEC_AAC_LATM == dec_type)
	{
		ret=__load_aac_table();
        if (ret == 0) {
            if(CSDRV_AUD_DEC_AAC== dec_type)
                ret=__csdrv_load_aac_extra_part_firmware();
            else
                ret=__csdrv_load_aac_extra_latm_firmware();
        }
        if (ret != 0){
            printk("Load AAC Firmware Failed!\n");
            return -1;
        }
		audio_writel(AUD_MAILBOX4, 0);
		audio_writel(AUD_MAILBOX5, AUD_HEAAC_TABLE_REGION);
	}

	/* add by ying 2008.10.07*/
	if (CSDRV_AUD_DEC_AC3 == dec_type)
	{
		ret=__load_eac3_table();
        if (ret == 0) {
            ret=__csdrv_load_eac3_extra_firmware();
        }
        if (ret != 0){
            printk("Load AC3 Firmware Failed!\n");
            return -1;
        }
		audio_writel(AUD_MAILBOX4, 0);
		audio_writel(AUD_MAILBOX5, AUD_EAC3_TABLE_REGION);
	}
#else
	memset((unsigned char *)csdrv_audio_ext_data_base, 0, AUD_EXT_DATA_SIZE);

	if (CSDRV_AUD_DEC_DTS == dec_type)
	{
		ret = request_firmware(&audio_data_fw, "audio_dts_extdata.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load DTS firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_dts_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load DTS firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
		__load_table_to_ram(csdrv_audio_ext_code_base, audio_cmd_fw->data, audio_cmd_fw->size);
		__load_table_to_ram(csdrv_audio_ext_data_base, audio_data_fw->data,audio_data_fw->size);
    		
		release_firmware(audio_cmd_fw); audio_cmd_fw = NULL;
        release_firmware(audio_data_fw);audio_data_fw= NULL;
	}
	else if(CSDRV_AUD_DEC_AAC == dec_type)
	{
		ret = request_firmware(&audio_data_fw, "audio_eaacv2_extdata.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load DTS firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_aacadts_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load DTS firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
		__load_table_to_ram(csdrv_audio_ext_code_base, audio_cmd_fw->data, audio_cmd_fw->size);
		__load_table_to_ram(csdrv_audio_ext_data_base, audio_data_fw->data, audio_data_fw->size);
    		
		release_firmware(audio_cmd_fw); audio_cmd_fw = NULL;
        release_firmware(audio_data_fw);audio_data_fw= NULL;
	}
	else if (CSDRV_AUD_DEC_AAC_LATM == dec_type)
	{
		ret = request_firmware(&audio_data_fw, "audio_eaacv2_extdata.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load AACLATM firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_aaclatmddr_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load AACLATM firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}

		__load_table_to_ram(csdrv_audio_ext_code_base, audio_cmd_fw->data, audio_cmd_fw->size);
		__load_table_to_ram(csdrv_audio_ext_data_base, audio_data_fw->data, audio_data_fw->size);
		
		release_firmware(audio_cmd_fw); audio_cmd_fw = NULL;
        release_firmware(audio_data_fw);audio_data_fw= NULL;
	}
	else if (CSDRV_AUD_DEC_MPA == dec_type)
	{
		ret = request_firmware(&audio_data_fw, "audio_mp2_extdata.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_data_fw == NULL ) {
			printk(KERN_ERR "Failed to load MP2 firmware data section\n");
			return -1;
		}
		ret = request_firmware(&audio_cmd_fw, "audio_mp2_cmd.bin", &(audio_pdev->dev));
		if (ret != 0 || audio_cmd_fw == NULL ) {
			printk(KERN_ERR "Failed to load MP2 firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
			return -1;
		}
		__load_table_to_ram(csdrv_audio_ext_code_base, audio_cmd_fw->data, audio_cmd_fw->size);
		__load_table_to_ram(csdrv_audio_ext_data_base, audio_data_fw->data, audio_data_fw->size);
    		
		release_firmware(audio_cmd_fw); audio_cmd_fw = NULL;
        release_firmware(audio_data_fw);audio_data_fw= NULL;
	}
#ifdef SUPPORT_EAC3_1201    
    else if (CSDRV_AUD_DEC_AC3 == dec_type)
    {
        ret = request_firmware(&audio_data_fw, "audio_eac3_extdata.bin", &(audio_pdev->dev));
        if (ret != 0 || audio_data_fw == NULL ) {
            printk(KERN_ERR "Failed to load EAC3 firmware data section\n");
            return -1;
        }
        ret = request_firmware(&audio_cmd_fw, "audio_eac3_cmd.bin", &(audio_pdev->dev));
        if (ret != 0 || audio_cmd_fw == NULL ) {
            printk(KERN_ERR "Failed to load EAC3 firmware CMD section\n");
            release_firmware(audio_data_fw);
            audio_data_fw = NULL;
            return -1;
        }
        __load_table_to_ram(csdrv_audio_ext_code_base, audio_cmd_fw->data, audio_cmd_fw->size);
        __load_table_to_ram(csdrv_audio_ext_data_base, audio_data_fw->data, audio_data_fw->size);
            
        release_firmware(audio_cmd_fw); audio_cmd_fw = NULL;
        release_firmware(audio_data_fw);audio_data_fw= NULL;
    }
#endif    

	audio_writel(AUD_MAILBOX5, AUD_EXT_DATA_REGION);
#endif

	return 0;
}


int __csdrv_aud_set_freq(unsigned int audio_freq)
{
#if defined(CONFIG_ARCH_ORION_CSM1200)
	unsigned short regval;
	unsigned int pll_freq;

	pll_freq = audio_readl(AUD_PLL_REG);

	pll_freq = 27000 / ((pll_freq >> 9) & 0x1f) * (pll_freq & 0xff);

	switch (audio_freq) {
	case AUD_SAMPLE_RATE_96KHZ:
		audio_freq = 96000;
		break;
	case AUD_SAMPLE_RATE_88_200KHZ:
		audio_freq = 88200;
		break;
	case AUD_SAMPLE_RATE_64KHZ:
		audio_freq = 64000;
		break;
	case AUD_SAMPLE_RATE_32KHZ:
		audio_freq = 32000;
		break;
	case AUD_SAMPLE_RATE_24KHZ:
		audio_freq = 24000;
		break;
	case AUD_SAMPLE_RATE_22_050KHZ:
		audio_freq = 22050;
		break;
	case AUD_SAMPLE_RATE_16KHZ:
		audio_freq = 16000;
		break;
	case AUD_SAMPLE_RATE_12KHZ:
		audio_freq = 12000;
		break;
	case AUD_SAMPLE_RATE_11_025KHZ:
		audio_freq = 11025;
		break;
	case AUD_SAMPLE_RATE_8KHZ:
		audio_freq = 8000;
		break;
	case AUD_SAMPLE_RATE_44_100KHZ:
		audio_freq = 44100;
		break;
	case AUD_SAMPLE_RATE_48KHZ:
		audio_freq = 48000;
		break;
	default:
		break;
}

{
        unsigned short HPD = (unsigned short) ((pll_freq * 1000L) / (audio_freq * 256L * 2L));
        unsigned short FREQ = (unsigned short) ((20L * audio_freq) / 1000L);
        unsigned int NEW_ERROR = (pll_freq * 10) - ((unsigned int) FREQ * (unsigned int) HPD) * 256L;

#ifdef DRV_AUDIO_DEBUG
        printk(KERN_INFO"pll_freq = %d HPD = %d NEW_ERROR=%d\n", pll_freq, HPD, NEW_ERROR);
#endif

        audio_writew(AUD_CLK_GEN_HPD, HPD);
        audio_writew(AUD_CLK_GEN_FREQ, FREQ);
        audio_writew(AUD_CLK_GEN_JITTER_LO, (unsigned short) (NEW_ERROR & 0xFFFF));

        regval = audio_readl(AUD_CLK_GEN_JITTER_HI);
        regval = (regval & 0xFFF8) | ((NEW_ERROR >> 16) & 0x7);
        audio_writew(AUD_CLK_GEN_JITTER_HI, regval);
}
#endif

#if defined(CONFIG_ARCH_ORION_CSM1201)
	unsigned short regval;
	unsigned int pll_freq;

	g_audio_currnet_freq = audio_freq;
	regval = audio_readl(AUD_CLK_GEN_JITTER_HI);

	if (regval & 0x4){ // JITTER_HI b2=1	
		switch(audio_freq){
		case AUD_SAMPLE_RATE_64KHZ:
		case AUD_SAMPLE_RATE_32KHZ: // true
		case AUD_SAMPLE_RATE_16KHZ:
		case AUD_SAMPLE_RATE_8KHZ:
			audio_freq = 32000;
			audio_writew( AUD_CLK_GEN_HPD, 17 );
			audio_writew( AUD_CLK_GEN_FREQ, 1920 );
			audio_writew( AUD_CLK_GEN_JITTER_LO, 0x5600);
			regval = (regval & 0xFFFc);
			audio_writew( AUD_CLK_GEN_JITTER_HI, regval);
			break;

		case AUD_SAMPLE_RATE_88_200KHZ:
		case AUD_SAMPLE_RATE_44_100KHZ: // true
		case AUD_SAMPLE_RATE_22_050KHZ:
		case AUD_SAMPLE_RATE_11_025KHZ:
			audio_freq = 44100;
			audio_writew( AUD_CLK_GEN_HPD, 12 );
			audio_writew( AUD_CLK_GEN_FREQ, 1764 );
			audio_writew( AUD_CLK_GEN_JITTER_LO, 0x3400);
			regval = (regval & 0xFFFc) | 0x1;
			audio_writew( AUD_CLK_GEN_JITTER_HI, regval);
			break;

		case AUD_SAMPLE_RATE_96KHZ:
		case AUD_SAMPLE_RATE_48KHZ: // true
		case AUD_SAMPLE_RATE_24KHZ:
		case AUD_SAMPLE_RATE_12KHZ:
			audio_freq = 48000;
			audio_writew( AUD_CLK_GEN_HPD, 11 );
			audio_writew( AUD_CLK_GEN_FREQ, 1920 );
			audio_writew( AUD_CLK_GEN_JITTER_LO, 0x6400 );
			regval = (regval & 0xFFFc) | 0x1;
			audio_writew( AUD_CLK_GEN_JITTER_HI, regval);
			break;

		default:
			printk("warning: only 48kHz, 44.1kHz, 32kHz accepted!\n");
			break;
		}     
	}
	else{	// JITTER_HI b2=0	
		pll_freq = audio_readl(AUD_PLL_REG);		
		pll_freq = 27000 / ((pll_freq >> 9) & 0x1f) * (pll_freq & 0xff);

		switch (audio_freq) {
		case AUD_SAMPLE_RATE_96KHZ:
			audio_freq = 96000;
			break;
		case AUD_SAMPLE_RATE_88_200KHZ:
			audio_freq = 88200;
			break;
		case AUD_SAMPLE_RATE_64KHZ:
			audio_freq = 64000;
			break;
		case AUD_SAMPLE_RATE_32KHZ:
			audio_freq = 32000;
			break;
		case AUD_SAMPLE_RATE_24KHZ:
			audio_freq = 24000;
			break;
		case AUD_SAMPLE_RATE_22_050KHZ:
			audio_freq = 22050;
			break;
		case AUD_SAMPLE_RATE_16KHZ:
			audio_freq = 16000;
			break;
		case AUD_SAMPLE_RATE_12KHZ:
			audio_freq = 12000;
			break;
		case AUD_SAMPLE_RATE_11_025KHZ:
			audio_freq = 11025;
			break;
		case AUD_SAMPLE_RATE_8KHZ:
			audio_freq = 8000;
			break;
		case AUD_SAMPLE_RATE_44_100KHZ:
			audio_freq = 44100;
			break;
		case AUD_SAMPLE_RATE_48KHZ:
			audio_freq = 48000;
			break;
		default:
			break;
		}

		{		
	        unsigned short HPD = (unsigned short) ((pll_freq * 1000L) / (audio_freq * 256L * 2L));
	        unsigned short FREQ = (unsigned short) ((20L * audio_freq) / 1000L);
	        unsigned int NEW_ERROR = (pll_freq * 10) - ((unsigned int) FREQ * (unsigned int) HPD) * 256L;

	        audio_writew(AUD_CLK_GEN_HPD, HPD);
	        audio_writew(AUD_CLK_GEN_FREQ, FREQ);
	        audio_writew(AUD_CLK_GEN_JITTER_LO, (unsigned short) (NEW_ERROR & 0xFFFF));

	        regval = audio_readl(AUD_CLK_GEN_JITTER_HI);
	        regval = (regval & 0xFFF8) | ((NEW_ERROR >> 16) & 0x7);
	        audio_writew(AUD_CLK_GEN_JITTER_HI, regval);
		}
	}
#endif

return 0;
}

static int __csdrv_aud_set_drc(unsigned int is_enable)
{
	unsigned int regval;

	regval = audio_readl(AUD_MAILBOX3);
	if (is_enable == 0)
		regval = regval & (~DRC_SET);
	else
		regval = regval | DRC_SET;

	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | DRC_SET);
	return 0;
}

static int __csdrv_aud_set_mute(unsigned int is_enable)
{
	unsigned int regval = 0;

	regval = audio_readl(AUD_MAILBOX3);
	if (is_enable == 0)
		regval = regval & (~MUTE_SET);
	else
		regval = regval | MUTE_SET;
	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | MUTE_SET);
	return 0;
}

static int __csdrv_aud_set_surround(unsigned int is_enable)
{
	unsigned int regval = 0;

	regval = audio_readl(AUD_MAILBOX3);
	if (is_enable == 0)
		regval = regval & (~SUR_SET);
	else
		regval = regval | SUR_SET;
	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | SUR_SET);
	return 0;
}

static int __csdrv_aud_set_input_mode(CSDRV_AUD_INPUT_MODE input_mode)
{
	unsigned int regval = 0;

	regval = audio_readl(AUD_MAILBOX3);
	if (input_mode == CSDRV_AUD_INPUT_NOBLOCK)
		regval = regval & (~INPUT_MODE_SET);
	else
		regval = regval | INPUT_MODE_SET;
	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | INPUT_MODE_SET);
        
	return 0;
}
static int __csdrv_aud_set_pts_sync(unsigned int is_enable)
{
	unsigned int regval = 0;

	regval = audio_readl(AUD_MAILBOX3);
	if (is_enable == 0)
		regval = regval & (~PTS_SET);
	else
		regval = regval | PTS_SET;
	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | PTS_SET);
	return 0;
}

static int __csdrv_aud_set_volume(CSDRV_AUD_Volume * vol)
{
	unsigned int regval = 0, main_vol = 0;

       /*just support 1 channel */
	main_vol += vol->front_left;
	main_vol += vol->front_right;
	main_vol += vol->rear_left;
	main_vol += vol->rear_right;
	main_vol += vol->center;
	main_vol /= 5;

	if (main_vol > 70)
		main_vol = 70;
	if (main_vol < 0)
		main_vol = 0;

	regval = audio_readl(AUD_MAILBOX3);
	regval &= 0xFF01FFFF;
	regval |= (main_vol << 17);
	audio_writel(AUD_MAILBOX3, regval);

	regval = audio_readl(AUD_MAILBOX2);
	audio_writel(AUD_MAILBOX2, regval | VOLUME_SET);
	return 0;
}

static int __csdrv_aud_play(void)
{
	audio_writel(AUD_RISC_CTRL, 0x0 | (AUD_REGION >> 3));
	audio_writel(AUD_ENABLE, 0x01);
	audio_writel(AUD_PTS_ENABLE, 0x0);

        return 0;
}

static int __csdrv_aud_stop(void)
{
        audio_writel(AUD2VID_MAILBOX, 0x0);
	audio_writel(AUD_ENABLE, 0x0);
	audio_writel(AUD_PTS_ENABLE, 0x0);
	//audio_writel(AUD_RISC_CTRL, 1 << 25);
	 while(audio_readl(AUD_IFIFOCNT))
       audio_readl(AUD_IFIFO);

	return 0;
}

static int __csdrv_aud_reset(void)
{
	unsigned int regval;

	//disable firmware
	audio_writel(AUD_RISC_CTRL, 1 << 25);

//ssleep(2);
	// wait the dma finish
	regval = audio_readl(AUD_DMA_STAT);
	while((regval & 0x2))// 0x2 busy flag
	{
		regval = audio_readl(AUD_DMA_STAT);
	}
//ssleep(2);
	//regval = audio_readl(AUD_FREQ_CTRL);
	//audio_writel(AUD_FREQ_CTRL, regval & (~0X200));

	//audio_writel(AUD_SPDIF_I2S_CMD, 0x2);
	//udelay(1000);
	//mdelay(500);
	//reset audio
//	regval = audio_readl(IDCS_RESET_REG);
//	audio_writel(IDCS_RESET_REG, regval & 0xFFFFFFF7);
//	udelay(10);
//	audio_writel(IDCS_RESET_REG, regval | 0x08);
//ssleep(2);
	//init cab buffer
	audio_writel(AUD_CABUP, CAB_UP_ADDR);
	audio_writel(AUD_CABLOW, CAB_LOW_ADDR);
	audio_writel(AUD_CABRP, CAB_LOW_ADDR);
//ssleep(2);
	//init pts buffer
	audio_writel(AUD_PTSUP, PTS_AUDIO_UP_ADDR);
	audio_writel(AUD_PTSLOW, PTS_AUDIO_LOW_ADDR);
	audio_writel(AUD_PTSRP, PTS_AUDIO_LOW_ADDR);
//ssleep(2);
	//
	audio_writel(AUD_SHITCNT, 0xf);	//set shift counter
	audio_writel(AUD_MAILBOX3, 0);	//clear Status register
	audio_writel(AUD_MAILBOX2, 0);	//clear Host_Request
//ssleep(2);
	__csdrv_aud_set_freq(AUD_SAMPLE_RATE_48KHZ);

	//regval = audio_readl(AUD_FREQ_CTRL);
	//audio_writel(AUD_FREQ_CTRL, regval | 0X200);

	__csdrv_aud_set_drc(0);
	__csdrv_aud_set_mute(0);
	__csdrv_aud_set_surround(0);
	__csdrv_aud_set_output_channel(AUD_PCM_STEREO);
	__csdrv_aud_set_volume(&audioinfo.volume);
	audioinfo.channel = AUD_PCM_STEREO;
	_mixer_init();

	//audio_writel(AUD_SPDIF_I2S_CMD, 0x3);
	
}

static int __csdrv_audio_pfm_reset(void)
{
	//init cab buffer
	audio_writel(AUD_CABUP, CAB_UP_ADDR);
	audio_writel(AUD_CABLOW, CAB_LOW_ADDR);
	audio_writel(AUD_CABRP, CAB_LOW_ADDR);
	xport_writel(0x410, CAB_LOW_ADDR << 1);	//xport_aud_cab_wp:0x410
	//init pts buffer
	audio_writel(AUD_PTSUP, PTS_AUDIO_UP_ADDR);
	audio_writel(AUD_PTSLOW, PTS_AUDIO_LOW_ADDR);
	audio_writel(AUD_PTSRP, PTS_AUDIO_LOW_ADDR);
        xport_writel(0x418, PTS_AUDIO_LOW_ADDR<<1);//xport_aud_pts_wp:0x418
	
	return 0;
}

/**********************End of internal function**************************/

static int csdrv_audio_open(struct inode *inode, struct file *file)
{
// #if !defined(CONFIG_ARCH_ORION_CSM1201)
// 	__csdrv_load_extra_firmware();
// #endif
	
	return 0;
}

static int csdrv_audio_release(struct inode *inodp,struct file *filp)
{
#if !defined(CONFIG_ARCH_ORION_CSM1201)
    if (audio_aac_part_data != NULL) {
        release_firmware(audio_aac_part_data);
        audio_aac_part_data = NULL;
    }
    if (audio_aac_part_cmd != NULL) {
        release_firmware(audio_aac_part_cmd);
        audio_aac_part_cmd = NULL;
    }   
    if (audio_aac_latm_cmd != NULL) {
        release_firmware(audio_aac_latm_cmd);
        audio_aac_latm_cmd = NULL;
    }   
    if (audio_aac_latm_data != NULL) {
        release_firmware(audio_aac_latm_data);
        audio_aac_latm_data = NULL;
    }   
    if (audio_sbr_part_data != NULL) {
        release_firmware(audio_sbr_part_data);
        audio_sbr_part_data = NULL;
    }   
    if (audio_sbr_part_cmd != NULL) {
        release_firmware(audio_sbr_part_cmd);
        audio_sbr_part_cmd = NULL;
    }   
    if (audio_ac3_part_data != NULL) {
        release_firmware(audio_ac3_part_data);
        audio_ac3_part_data = NULL;
    }   
    if (audio_ac3_part_cmd != NULL) {
        release_firmware(audio_ac3_part_cmd);
        audio_ac3_part_cmd = NULL;
    }   
    if (audio_eac3_part_data != NULL) {
        release_firmware(audio_eac3_part_data);
        audio_eac3_part_data = NULL;
    }   
    if (audio_eac3_part_cmd != NULL) {
        release_firmware(audio_eac3_part_cmd);
        audio_eac3_part_cmd = NULL;
    }   

    if (audio_dtsdata1 != NULL ){
        release_firmware(audio_dtsdata1);
        audio_dtsdata1 = NULL;
    }

    if (audio_dtsdata2 != NULL ){
        release_firmware(audio_dtsdata2);
        audio_dtsdata2 = NULL;
    }

    if (audio_dtsdata3 != NULL ){
        release_firmware(audio_dtsdata3);
        audio_dtsdata3 = NULL;
    }
    
    if (longwin  != NULL) {
        release_firmware(longwin);
        longwin = NULL;
    }

    if (tabsbr != NULL) {
        release_firmware(tabsbr);
        tabsbr = NULL;
    }

    if (taba != NULL){
        release_firmware(taba);
        taba = NULL;
    }
    if (tabs != NULL){
        release_firmware(tabs);
        tabs = NULL;
    }

    if (ntab != NULL){
        release_firmware(ntab);
        ntab = NULL;
    }

    if (eac3_vq != NULL){
        release_firmware(eac3_vq);
        eac3_vq = NULL;
    }

#endif

    if (audio_cmd_fw != NULL){
        release_firmware(audio_cmd_fw);
        audio_cmd_fw = NULL;
    }

    if (audio_data_fw != NULL){
        release_firmware(audio_data_fw);
        audio_data_fw = NULL;
    }
    return 0;
}

static int csdrv_audio_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned long flags;
	unsigned int reg_val = 0;
    int ret;

	switch (cmd) {
	case CSAUD_IOC_SET_RESET:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_reset();
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_PLAY:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_play();
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_STOP:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_stop();
                __csdrv_audio_pfm_reset();
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_MUTE:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(reg_val, (unsigned int __user *) arg);
		__csdrv_aud_set_mute(reg_val);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_VOLUME:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		copy_from_user(&audioinfo.volume, (CSDRV_AUD_Volume __user *) arg, sizeof(CSDRV_AUD_Volume));
		__csdrv_aud_set_volume(&audioinfo.volume);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_DECTYPE:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(reg_val, (unsigned int __user *) arg);
		ret=__csdrv_aud_set_dec(reg_val);
		audioinfo.decoder_type = reg_val;
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
        if (ret != 0)
            return -EINVAL;
		break;
	case CSAUD_IOC_SET_FREQ:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(reg_val, (unsigned int __user *) arg);
		__csdrv_aud_set_freq(reg_val);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_DRC:	/*ac3 only */
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(reg_val, (unsigned int __user *) arg);
		__csdrv_aud_set_drc(reg_val);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_SURROUND:	/*ac3 only */
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(reg_val, (unsigned int __user *) arg);
		__csdrv_aud_set_surround(reg_val);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_PTS_SYNC:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(reg_val, (unsigned int __user *) arg);
		__csdrv_aud_set_pts_sync(reg_val);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_MIXER_STATUS:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(audioinfo.mixer.mixer_status, (unsigned int __user *) arg);
                if(audioinfo.mixer.mixer_status){
                    _mixer_init();
                    audioinfo.mixer.mixer_status = 1;
                }
		__csdrv_aud_set_mixer_status(audioinfo.mixer.mixer_status);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_MIXER_CONFIG:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		copy_from_user(&audioinfo.mixer.mixer_config, (CSDRV_AUD_MixerConfig __user *) arg,
			       sizeof(CSDRV_AUD_MixerConfig));
		__csdrv_aud_set_mixer_config(&audioinfo.mixer.mixer_config);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_OUTPUT_CHANNEL:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(audioinfo.channel, (unsigned int __user *) arg);
		__csdrv_aud_set_output_channel(audioinfo.channel);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_OUTPUT_CHANNEL:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audioinfo.channel, (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_PLAYMODE:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_set_playmode(arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_STARTDELAY:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(audioinfo.startdelay, (unsigned int __user *) arg);
		__csdrv_aud_set_startdelay(audioinfo.startdelay);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_DECODEDBYTES:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(audioinfo.decodedbytes, (unsigned int __user *) arg);
		__csdrv_aud_set_decodedbytes(audioinfo.decodedbytes);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_EQUALIZER_STATUS:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(audioinfo.equalizer_status, (unsigned int __user *) arg);
		__csdrv_aud_set_equalizer_status(audioinfo.equalizer_status);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_EQUALIZER_CONFIG:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		copy_from_user(&audioinfo.equalizer_config, (CSDRV_AUD_EqualizerConfig __user *) arg,
			       sizeof(CSDRV_AUD_EqualizerConfig));
		//__csdrv_aud_set_equalizer_config(&audioinfo.equalizer_config);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_DECTYPE:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_get_queryinfo(CSDRV_AUD_DECODERTYPE, (void __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_BITRATE:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_get_queryinfo(CSDRV_AUD_BITRATE, (void __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_SAMPLERATE:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_get_queryinfo(CSDRV_AUD_SAMPLERATE, (void __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_ACMODE:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_get_queryinfo(CSDRV_AUD_ACMODE, (void __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_DECODEDBYTES:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_get_queryinfo(CSDRV_AUD_DECODEDBYTES, (void __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_DECODEDFRAME:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_get_queryinfo(CSDRV_AUD_DECODEDFRAME, (void __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_VOLUME:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__copy_to_user((CSDRV_AUD_Volume __user *) arg, &audioinfo.volume, sizeof(CSDRV_AUD_Volume));
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_MIXER_CONFIG:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__copy_to_user((CSDRV_AUD_MixerConfig __user *) arg, &audioinfo.mixer.mixer_config,
			       sizeof(CSDRV_AUD_MixerConfig));
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_EQUALIZER_CONFIG:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__copy_to_user((CSDRV_AUD_EqualizerConfig __user *) arg, &audioinfo.equalizer_config,
			       sizeof(CSDRV_AUD_EqualizerConfig));
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_WRITE_MIXER_BUFFER:
		{
			unsigned int temp_arg[2];
			spin_lock_irqsave(&csdrv_audio_lock, flags);
			__copy_from_user(temp_arg, (unsigned int __user *) arg, 8);
#ifdef DRV_AUDIO_DEBUG
			printk("write mixer buffer 0x%x,%d\n", temp_arg[0], temp_arg[1]);
#endif
			__csdrv_aud_write_mixer_buffer((char *) temp_arg[0], temp_arg[1]);
			spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		}
		break;
	case CSAUD_IOC_SET_OUTPUT_DEV:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(audioinfo.outputdev, (CSDRV_AUD_OUTPUTDEV __user *) arg);
                __csdrv_aud_set_outputdev(audioinfo.outputdev);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_OUTPUT_DEV:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audioinfo.outputdev, (CSDRV_AUD_OUTPUTDEV __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_SET_I2SFORMAT:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(audioinfo.i2sformat, (unsigned short __user *) arg);
		__csdrv_aud_set_i2sformat();
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_I2SFORMAT:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audioinfo.i2sformat, (unsigned short __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

	case CSAUD_IOC_SET_ERROR_LEVEL:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_aud_set_error_level(arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_ERROR_LEVEL:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		//__put_user(audioinfo.i2sformat, (unsigned short __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

	case CSAUD_IOC_PFM_GETCAB_ADDR:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(CAB_REGION, (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_PFM_GETCAB_SIZE:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(CAB_SIZE, (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_PFM_GETCAB_RP:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audio_readl(AUD_CABRP), (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_PFM_GETCAB_WP:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audio_readl(AUD_CABWP), (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_PFM_GETCAB_LOW:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audio_readl(AUD_CABLOW), (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_PFM_GETCAB_UP:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audio_readl(AUD_CABUP), (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_PFM_SETXPORT_CABWP:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(reg_val, (unsigned int __user *) arg);
		xport_writel(0x410, reg_val);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

	case CSAUD_IOC_PFM_GETOFIFO_COUNT:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(reg_val, (unsigned int __user *) arg);
		__put_user(audio_readl(AUD_OFIFOCNT), (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;		
		
	case CSAUD_IOC_PFM_RESET:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__csdrv_audio_pfm_reset();
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

	case CSAUD_IOC_SET_BALANCE:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		audioinfo.equalizer_config.balance = arg;
		reg_val = audio_readl(AUD_MAILBOX2);
		audio_writel(AUD_MAILBOX2, reg_val | CHANNEL_BALANCE);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
	case CSAUD_IOC_GET_BALANCE:
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audioinfo.equalizer_config.balance, (unsigned int __user *)arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

        case CSAUD_IOC_PFM_GETPTS_SIZE:
                spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(AUD_PTS_SIZE, (unsigned int __user *)arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

        case CSAUD_IOC_PFM_GETPTS_ADDR:
                spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(AUD_PTS_REGION, (unsigned int __user *)arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

        case CSAUD_IOC_PFM_GETPTS_LOW:
                spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audio_readl(AUD_PTSLOW), (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
        
        case CSAUD_IOC_PFM_GETPTS_UP:
                spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audio_readl(AUD_PTSUP), (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

        case CSAUD_IOC_PFM_GETPTS_WP:
                spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(xport_readl(0x418), (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
        
        case CSAUD_IOC_PFM_GETPTS_RP:
                spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audio_readl(AUD_PTSRP), (unsigned int __user *) arg);
                spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
        
        case CSAUD_IOC_PFM_SETPTS_WP:
                spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(reg_val, (unsigned int __user *) arg);
        	xport_writel(0x418, reg_val);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

        case CSAUD_IOC_SET_INPUTMODE:
                spin_lock_irqsave(&csdrv_audio_lock, flags);
		__get_user(audioinfo.input_mode, (unsigned int __user *) arg);
                __csdrv_aud_set_input_mode(audioinfo.input_mode);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

        case CSAUD_IOC_GET_INPUTMODE:
                spin_lock_irqsave(&csdrv_audio_lock, flags);
		__put_user(audioinfo.input_mode, (unsigned int __user *) arg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
        
        default:
		return -EINVAL;
	}
	return 0;
}

static unsigned int csdrv_audio_poll(struct file *filp, poll_table * wait)
{
	unsigned int mask = 0;
#ifdef DRV_AUDIO_DEBUG
	printk("audio poll run!\n");
#endif
	poll_wait(filp, &error_level_queue, wait);

	if (errorlevel_flag) {
		spin_lock_irq(error_level_lock);
		errorlevel_flag = 0;
		spin_unlock_irq(error_level_lock);
		mask = POLLIN | POLLWRNORM;
	}
	return mask;
}

/*
 * Architectures vary in how they handle caching for addresses
 * outside of main memory.
 *
 */
static inline int uncached_access(struct file *file, unsigned long addr)
{

	/*
	 * Accessing memory above the top the kernel knows about or through a file pointer
	 * that was marked O_SYNC will be done non-cached.
	 */
	if (file->f_flags & O_SYNC)
		return 1;
	return addr >= __pa(high_memory);

}

static int csdrv_audio_mmap(struct file *file, struct vm_area_struct *vma)
{
#if defined(__HAVE_PHYS_MEM_ACCESS_PROT)
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;

	vma->vm_page_prot = phys_mem_access_prot(file, offset, vma->vm_end - vma->vm_start, vma->vm_page_prot);
#elif defined(pgprot_noncached)
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	int uncached;

	uncached = uncached_access(file, offset);
	if (uncached)
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;
	return 0;
}

static struct file_operations csdrv_audio_fops = {
	.owner = THIS_MODULE,
	.open  = csdrv_audio_open,
	.ioctl = csdrv_audio_ioctl,
	.poll = csdrv_audio_poll,
	.mmap = csdrv_audio_mmap,
    .release = csdrv_audio_release
};

static struct miscdevice csdrv_audio_miscdev = {
	        MISC_DYNAMIC_MINOR,
	        "csaudio",
	        &csdrv_audio_fops
};

irqreturn_t csdrv_aud_irq(int irq, void *dev_id, struct pt_regs *egs)
{
	unsigned long flags;
	unsigned int irq_reg;

	irq_reg = audio_readl(AUD_MAILBOX4);
	switch (irq_reg) {
	case AUD_REQ_STARTDELAY:
#ifdef DRV_AUDIO_DEBUG
		printk("interrupt:AUD_REQ_STARTDELAY\n");
		printk("audioinfo.startdelay %d\n", audioinfo.startdelay);
#endif
		audio_writel(AUD_MAILBOX5, audioinfo.startdelay);
		break;

	case AUD_REQ_DECODED_BYTES:
#ifdef DRV_AUDIO_DEBUG
		printk("interrupt:AUD_REQ_DECODED_BYTES\n");
		printk("audioinfo.decodedbytes %d\n", audioinfo.decodedbytes);
#endif
		audio_writel(AUD_MAILBOX5, audioinfo.decodedbytes);
		break;

/*a. information from audio firmware to host*/
	case AUD_WATCHDOG:
		irq_reg = audio_readl(AUD_MAILBOX5);
#ifdef DRV_AUDIO_DEBUG
		printk("interrupt:AUD_WATCHDOG\n");
		printk("interrupt: decoded frames number is %d\n", irq_reg);
#endif
		break;

	case AUD_SAMPLE_RATE_INT:
		audioinfo.samplerate = audio_readl(AUD_MAILBOX5);
#ifdef DRV_AUDIO_DEBUG
		printk("interrupt:AUD_SAMPLE_RATE_INT\n");
		printk("interrupt: current sample rate is %d\n", audioinfo.samplerate);
#endif
		__csdrv_aud_set_freq(audioinfo.samplerate);
		break;
  
	case AUD_MIX_READ_WP:
#ifdef DRV_AUDIO_DEBUG
		printk("interrupt:AUD_MIX_READ_WP\n");
#endif
                spin_lock_irqsave(&csdrv_audio_lock, flags);
#ifdef FIX_MIX_POINTERS
        audio_writel(AUD_MAILBOX5,
                 ((audioinfo.mixer.wp_offset + AUD_MIX_REGION) >> 5) | (audioinfo.mixer.wp_toggle << 31));
#else
		audio_writel(AUD_MAILBOX5,
			     ((audioinfo.mixer.wp_offset + AUD_REGION) >> 5) | (audioinfo.mixer.wp_toggle << 31));
#endif
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
#ifdef DRV_AUDIO_DEBUG
		printk("interrupt: current DDR Write pointer is 0x%x, offset = 0x%x, toggle = 0x%x\n",
		       audioinfo.mixer.ddr_wp, audioinfo.mixer.wp_offset, audioinfo.mixer.wp_toggle);

#endif
		break;
 
	case AUD_MIX_WRITE_RP:
#ifdef DRV_AUDIO_DEBUG
		printk("interrupt:AUD_MIX_WRITE_RP\n");
#endif
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		irq_reg = audio_readl(AUD_MAILBOX5);

		audioinfo.mixer.rp_toggle = (irq_reg & MIX_BUFFER_TOGGLE31) >> 31;
#ifdef FIX_MIX_POINTERS
        audioinfo.mixer.rp_offset = (irq_reg << 5) - AUD_MIX_REGION; // toggle bit shift out.
#else
		audioinfo.mixer.rp_offset = (irq_reg << 5) - AUD_REGION;
#endif
 
		audioinfo.mixer.ddr_rp = (unsigned int)csdrv_audio_mix_buffer_base + (unsigned int)audioinfo.mixer.rp_offset;
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
#ifdef DRV_AUDIO_DEBUG
		printk("interrupt: current DDR read pointer is 0x%x, offset = 0x%x, toggle = 0x%x\n",
		       audioinfo.mixer.ddr_rp, audioinfo.mixer.rp_offset, audioinfo.mixer.rp_toggle);
#endif
		break;

	case AUD_ERROR:
#ifdef DRV_AUDIO_INFO
		printk("interrupt: AUD_ERROR------\n");
#endif
		irq_reg = audio_readl(AUD_MAILBOX5);
		switch (irq_reg) {
		case AUD_MIX_SR_ERROR:
#ifdef DRV_AUDIO_INFO
			printk("MIX_SAMPLERATE_ERROR\n");
#endif
			break;  
		case AUD_MIX_NO_DATA_ERROR:
#ifdef DRV_AUDIO_TESTINFO
			printk("MIX_NO_DATA_ERROR\n");
#endif
            spin_lock_irqsave(&csdrv_audio_lock, flags);
            audioinfo.mixer.mixer_status = 0;
            spin_unlock_irqrestore(&csdrv_audio_lock, flags);
			break;
		case AUD_EQUA_MODE_ERROR:
#ifdef DRV_AUDIO_INFO
			printk("EQUALIZE_MODE_ERROR\n");
#endif
			break;
		case AUD_KALAOK_SR_ERROR:
#ifdef DRV_AUDIO_INFO
			printk("KALAOK_SR_ERROR\n");
#endif
			break;
		case AUD_OUTPUT_CNANNEL_ERROR:
#ifdef DRV_AUDIO_INFO
			printk("OUTPUT_CNANNEL_ERROR\n");
#endif
			break;
		case AUD_ERROR_LEVEL:
			spin_lock_irq(error_level_lock);
			errorlevel_flag = 1;
			wake_up(&error_level_queue);
			spin_unlock_irq(error_level_lock);
#ifdef DRV_AUDIO_DEBUG
			printk("interrupt:AUD_ERROR_LEVEL\n");
#endif
			break;

		default:
#ifdef DRV_AUDIO_INFO
			printk("unknown error 0x%x\n", irq_reg);
#endif
			break;
		}
		break;
/*end of a.*/

/*b. mixer configuration*/
	case AUD_REQ_DDR_BASE:
#ifdef DRV_AUDIO_INFO
		printk("interrupt:AUD_REQ_DDR_BASE\n");
#endif
		spin_lock_irqsave(&csdrv_audio_lock, flags);
#ifdef FIX_MIX_POINTERS
#ifdef DRV_AUDIO_DEBUG
        printk("0x%x\n", AUD_MIX_REGION);
#endif
        audio_writel(AUD_MAILBOX5, AUD_MIX_REGION >> 5);
#else
#ifdef DRV_AUDIO_DEBUG
        printk("0x%x\n", AUD_REGION);
#endif
		audio_writel(AUD_MAILBOX5, AUD_REGION >> 5);
#endif
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;

	case AUD_REQ_DDR_RP_WP:
		{
			unsigned int r = 0, w = 0;
#ifdef DRV_AUDIO_INFO
			printk("interrupt:AUD_REQ_DDR_RP_WP\n");
#endif
			spin_lock_irqsave(&csdrv_audio_lock, flags);
			r = (audioinfo.mixer.rp_offset >> 5) | (audioinfo.mixer.rp_toggle << 15);
			w = (audioinfo.mixer.wp_offset >> 5) | (audioinfo.mixer.wp_toggle << 15);
#ifdef DRV_AUDIO_DEBUG
			printk("rp 0x%x, wp 0x%x\n", audioinfo.mixer.rp_offset, audioinfo.mixer.wp_offset);
			printk(" 0x%x\n", (r << 16) | w);
#endif
			audio_writel(AUD_MAILBOX5, ((r << 16) | w));
			spin_unlock_irqrestore(&csdrv_audio_lock, flags);
			break;
		}
	case AUD_REQ_DDR_BIT_SR_LEVEL_UP:
#ifdef DRV_AUDIO_INFO
		printk("interrupt:AUD_REQ_DDR_BIT_SR_LEVEL_UP\n");
#endif
#ifdef DRV_AUDIO_DEBUG
		printk("sample ratel %d\n", audioinfo.mixer.mixer_config.mixer_sample_rate);
		printk("sample level %d\n", audioinfo.mixer.mixer_config.mixer_level);
		printk("size%d\n", AUD_MIX_SIZE);
#endif
		spin_lock_irqsave(&csdrv_audio_lock, flags);
		irq_reg = AUD_MIX_SIZE >> 5;
		irq_reg |= (audioinfo.mixer.mixer_config.mixer_level << 16);
		irq_reg |= (audioinfo.mixer.mixer_config.mixer_sample_rate << 24);
		irq_reg |= 0x80000000;
#ifdef DRV_AUDIO_DEBUG
		printk("irq_reg = 0x%x\n", irq_reg);
#endif
		audio_writel(AUD_MAILBOX5, irq_reg);
		spin_unlock_irqrestore(&csdrv_audio_lock, flags);
		break;
/*end of b.*/

/*c. equalizer configuration*/
	case AUD_REQ_EQ_GAIN:
		{
			int gains = 0, tempbandweight = 0;
#ifdef DRV_AUDIO_INFO
			printk("interrupt:AUD_REQ_EQ_GAIN\n");
#endif
			for (gains = 0; gains < 10; gains++) {
				tempbandweight = abs(audioinfo.equalizer_config.equalizer_band_weight[gains]) << 27;
				if (audioinfo.equalizer_config.equalizer_band_weight[gains] < 0) {
					tempbandweight |= 0x80000000;
				}
				audio_writel(AUD_MAILBOX5, tempbandweight);
				irq_reg |= 0x80000000;
				audio_writel(AUD_MAILBOX4, irq_reg);
				if (gains == 9)
					break;
				while (irq_reg & 0x80000000) {
					irq_reg = audio_readl(AUD_MAILBOX4);
				}
			}
		}
		break;

	case AUD_CHANNEL_BALANCE:
#ifdef DRV_AUDIO_INFO
		printk("interrupt: AUD_CHANNEL_BALANCE %d\n", audioinfo.equalizer_config.balance);
#endif
		audio_writel(AUD_MAILBOX5, audioinfo.equalizer_config.balance);
		break;
/*end of c.*/

#if !defined(CONFIG_ARCH_ORION_CSM1201)
/*d. firmware switch*/
	case AUD_INT_REQUIRE_AAC:
#ifdef DRV_AUDIO_INFO
		printk("interrupt: AUD_INT_REQUIRE_AAC, current decoder type %d\n", audioinfo.decoder_type);
#endif			
		if (CSDRV_AUD_DEC_AAC == audioinfo.decoder_type)
		{
			__csdrv_aud_set_aac_part();
		}
		else if (CSDRV_AUD_DEC_AAC_LATM == audioinfo.decoder_type)
		{
			__csdrv_aud_set_aac_latm();
		}
		else
		{
			printk("interrupt: invalid decoder_type %d in AUD_INT_REQUIRE_AAC\n"
				, audioinfo.decoder_type);
		}
		break;

	case AUD_INT_REQUIRE_SBR:
#ifdef DRV_AUDIO_INFO
		printk("interrupt: AUD_INT_REQUIRE_SBR\n");
#endif			
       
        __csdrv_aud_set_sbr_part();
		break;

    case AUD_INT_REQUIRE_AC3:
#ifdef DRV_AUDIO_INFO
		printk("Interrupt: AUD_INT_REQUIRE_AC3\n");	
#endif
		__csdrv_aud_set_ac3_part();
		break;

    case AUD_INT_REQUIRE_EAC3:
#ifdef DRV_AUDIO_INFO
		printk("Interrupt: AUD_INT_REQUIRE_EAC3\n");
#endif
		__csdrv_aud_set_eac3_part();
		break;
#endif //CONFIG_ARCH_ORION_CSM1201

	default:
#ifdef DRV_AUDIO_INFO
		printk("interrupt: Unknow 0x%x\n", irq_reg);
		irq_reg = audio_readl(AUD_MAILBOX5);
		printk("interrupt:  0x%x\n", irq_reg);
#endif
		break;
	}
	irq_reg = audio_readl(AUD_INT);
	audio_writel(AUD_INT, irq_reg & 0xfffffffe);

	return IRQ_HANDLED;
}

static int __init csdrv_audio_init(void)
{
	int rtval = -EIO;
	
    	audio_pdev = platform_device_register_simple("audio_device", 0, NULL, 0);
    	if (IS_ERR(audio_pdev)) {
        	return -ENODEV;
    	}

	if (NULL == (csdrv_audio_reg_base = (unsigned char *) ioremap(CSDRV_AUD_REG_BASE, CSDRV_AUD_REG_SIZE)))
		goto INIT_ERR0;

	if (NULL == (csdrv_idcs_reg_base = (unsigned char *) ioremap(CSDRV_IDCS_REG_BASE, CSDRV_IDCS_REG_SIZE)))
		goto INIT_ERR1;

	if (NULL == (csdrv_audio_itcm_base = (unsigned char *) ioremap(CSDRV_AUD_ITCM_BASE, CSDRV_AUD_ITCM_SIZE)))
		goto INIT_ERR2;

	if (NULL == (csdrv_audio_dtcm_base = (unsigned char *) ioremap(CSDRV_AUD_DTCM_BASE, CSDRV_AUD_DTCM_SIZE)))
		goto INIT_ERR3;

	if (NULL ==
	    (csdrv_audio_risc_reg_base = (unsigned char *) ioremap(CSDRV_AUD_RISC_REG, CSDRV_AUD_RISC_REG_SIZE)))
		goto INIT_ERR4;
        
	if ((rtval = misc_register(&csdrv_audio_miscdev)))
                goto INIT_ERR5;

	if (NULL == (csdrv_audio_mix_buffer_base = (unsigned char *) ioremap(AUD_MIX_REGION, AUD_MIX_SIZE)))
		goto INIT_ERR6;

#if !defined(CONFIG_ARCH_ORION_CSM1201)
	if (NULL == (csdrv_audio_dts_table_base = (unsigned char *) ioremap(AUD_DTS_TABLE_REGION, AUD_DTS_TABLE_SIZE)))
		goto INIT_ERR7;

	if (NULL == (csdrv_audio_aac_table_base = (unsigned char *) ioremap(AUD_HEAAC_TABLE_REGION, AUD_HEAAC_TABLE_SIZE)))
		goto INIT_ERR8;

    if (NULL == (csdrv_audio_eac3_table_base = (unsigned char *) ioremap(AUD_EAC3_TABLE_REGION, AUD_EAC3_TABLE_SIZE)))
		goto INIT_ERR9;
#else
	if (NULL == (csdrv_audio_ext_code_base = (unsigned char *) ioremap(AUD_EXT_CODE_REGION, AUD_EXT_CODE_SIZE)))
		goto INIT_ERR10;
	if (NULL == (csdrv_audio_ext_data_base = (unsigned char *) ioremap(AUD_EXT_DATA_REGION, AUD_EXT_DATA_SIZE)))
		goto INIT_ERR11;
#endif
	if (request_irq(21, csdrv_aud_irq, SA_INTERRUPT, "cs_audio", NULL)) {
		printk(KERN_ERR "csdrv_aud: cannot register IRQ \n");
		return -EIO;
	}
	/*added jia.ma */
	__csdrv_aud_set_freq(AUD_SAMPLE_RATE_48KHZ);
	/*default volume*/
	audioinfo.volume.center = 70;
	audioinfo.volume.front_left = 70;
	audioinfo.volume.front_right = 70;
	audioinfo.volume.rear_left = 70;
	audioinfo.volume.rear_right = 70;
	audioinfo.volume.lfe = 70;

	printk(KERN_INFO "csdrv audio init ok ...\n");

	rtval = 0;
	return rtval;

#if defined(CONFIG_ARCH_ORION_CSM1201)
      INIT_ERR11:
	iounmap((void *) csdrv_audio_ext_data_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR11 ...\n");
	  INIT_ERR10:
	iounmap((void *) csdrv_audio_ext_code_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR10 ...\n");
#else	
      INIT_ERR9:
	iounmap((void *) csdrv_audio_eac3_table_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR9 ...\n");
      INIT_ERR8:
	iounmap((void *) csdrv_audio_aac_table_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR8 ...\n");
      INIT_ERR7:
	iounmap((void *) csdrv_audio_dts_table_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR7 ...\n");
#endif	
      INIT_ERR6:
	iounmap((void *) csdrv_audio_mix_buffer_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR6 ...\n");
      INIT_ERR5:
	iounmap((void *) csdrv_audio_risc_reg_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR5 ...\n");
      INIT_ERR4:
	iounmap((void *) csdrv_audio_dtcm_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR4...\n");
      INIT_ERR3:
	iounmap((void *) csdrv_audio_itcm_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR3 ...\n");
      INIT_ERR2:
	iounmap((void *) csdrv_idcs_reg_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR2 ...\n");
      INIT_ERR1:
	iounmap((void *) csdrv_audio_reg_base);
	printk(KERN_INFO "csdrv audio init INIT_ERR1 ...\n");
      INIT_ERR0:
	printk(KERN_INFO "csdrv audio init INIT_ERR0 ...\n");
	return rtval;
}

static void __exit csdrv_audio_exit(void)
{
	iounmap((void *) csdrv_audio_reg_base);
	iounmap((void *) csdrv_idcs_reg_base);
	iounmap((void *) csdrv_audio_itcm_base);
	iounmap((void *) csdrv_audio_dtcm_base);
	iounmap((void *) csdrv_audio_risc_reg_base);
#if !defined(CONFIG_ARCH_ORION_CSM1201)
	iounmap((void *) csdrv_audio_mix_buffer_base);
	iounmap((void *) csdrv_audio_dts_table_base);
	iounmap((void *) csdrv_audio_aac_table_base);
#else
	iounmap((void *) csdrv_audio_ext_code_base);
	iounmap((void *) csdrv_audio_ext_data_base);
#endif

	
	printk(KERN_INFO "csdrv audio exit...\n");
}

module_init(csdrv_audio_init);
module_exit(csdrv_audio_exit);
