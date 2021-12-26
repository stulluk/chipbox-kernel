#ifndef _ORIONRC_H_
#define _ORIONRC_H_

#define RC_INTS_L		(0x000)    /*Interrupt Status Register*/
#define RC_INTS_H		(0x002)
#define RC_INTM_L		(0x004)    /*Interrupt Mask Register*/
#define RC_INTM_H		(0x006)    
#define RC_CTL_L                (0x008)    /*RC Control Register*/
#define RC_CTL_H                (0x00a)    
#define RC_TIME_INTERVAL_L      (0x00c)    /*RC Time Interval Register*/
#define RC_TIME_INTERVAL_H      (0x00e)
#define RC_UHF_CTL_L            (0x010)    /*UHF control Register*/
#define RC_UHF_CTL_H            (0x012)
#define RC_UHF_TIME_INTERVAL_L  (0x014)    /*UHF Time Interval  Register*/
#define RC_UHF_TIME_INTERVAL_H	(0x016)
#define RC_INTR_CNT_L           (0x018)    /*RC Interrupt counter Register*/
#define RC_INTR_CNT_H           (0x01a)     
#define RC_UHF_INTR_CNT_L       (0x01c)    /*UHF Interrupt counter Register*/
#define RC_UHF_INTR_CNT_H       (0x01e)    
#define RC_FIFO1_L              (0x020)    /*RC FIFO1*/
#define RC_FIFO1_H              (0x022)
#define RC_FIFO1_CNT_L          (0x024)    /*RC FIFO1 count*/   
#define RC_FIFO1_CNT_H          (0x026)
#define RC_FIFO2_L              (0x028)    /*RC FIFO2*/
#define RC_FIFO2_H              (0x02a)
#define RC_FIFO2_CNT_L          (0x02c)    /*RC FIFO2 count*/   
#define RC_FIFO2_CNT_H          (0x02e)  
#define RC_UHF_FIFO1_L          (0x030)    /*UHF FIFO1 */
#define RC_UHF_FIFO1_H          (0x032)
#define RC_UHF_FIFO1_CNT_L      (0x034)    /*UHF FIFO1 Count*/
#define RC_UHF_FIFO1_CNT_H      (0x036)
#define RC_UHF_FIFO2_L          (0x038)    /*UHF FIFO2 */
#define RC_UHF_FIFO2_H          (0x03a)
#define RC_UHF_FIFO2_CNT_L      (0x03c)    /*UHF FIFO2 Count*/
#define RC_UHF_FIFO2_CNT_H      (0x03e)
#define RC_FIR_L                (0x040)    /*RC FIR Register*/
#define RC_FIR_H                (0x042)
#define RC_UHF_FIR_L            (0x044)    /*UHF FIR Register*/ 
#define RC_UHF_FIR_H            (0x046)
#define RC_LED_DATA_L           (0x200)    /*LED DATA Register*/
#define RC_LED_DATA_H           (0x202)
#define RC_LED_CNTL_L           (0x204)    /*LED Counter Register*/
#define RC_LED_CNTL_H           (0x206)    /*LED Counter Register*/
#define RC_KSCAN_DATA_L         (0x300)    /*KSCAN DATA Register*/
#define RC_KSCAN_DATA_H         (0x302)
#define RC_KSCAN_CNTL_L         (0x304)    /*KSCAN Counter Register*/
#define RC_KSCAN_CNTL_H         (0x306)
#define RC_KSCAN_ARB_CNTL_L     (0x308)    /*KSCAN Arbitrates Register*/ 
#define RC_KSCAN_ARB_CNTL_H     (0x30a)

#define ORION_RC_BASE		0x10172000
#define ORION_RC_SIZE 		0x1000
#define ORION_RC_IRQ		8

#define PCLK_MHz 		(PCLK_FREQ/1000000)
#define FILTER_PRECISION 	(450)

#define RC_CTL_OPTION 		( 7) 	/* rc_en | rc_pol | noise_filter_en */
#define BITTIME_CNT 		(PCLK_MHz)
#define PANNEL_SELECT 		(11) 	/* GPIO input for remote controller */

#define DRIVER_DESC		"ORION Remote Controller"

#define RC_MAGIC                'r'

/* Key number */
#define ORION_RC_KNUM 		KEY_MAX

/* Parse period */
#define HEAD_PERIOD		0
#define ADDR_PERIOD		1
#define CMD_PERIOD		2

#undef PDEBUG

//#define CONFIG_RC_DEBUG
static int RC_DEBUG = 0;
#ifdef CONFIG_RC_DEBUG
# ifdef __KERNEL__
#  define PDEBUG(fmt, args...) printk( fmt, ## args)
# else
#  define PDEBUG(fmt, args...) printf(fmt, ## args)	
# endif  
#else
# define PDEBUG(fmt, args...) \
	if (RC_DEBUG) printk( fmt, ## args)
#endif

/* Infrared Remote Control Protocol type */

typedef enum rc_protocal_ {
	NEC, RC5, RC6, 
#if 0
	RCMM, RECS80, RCA, XSAT, JVC, NRC17, SHARP, SIRC, ITT,
#endif
	P_TOTAL
} rc_protocal_t;

typedef struct rc_record_ {
	unsigned short addr;
	unsigned short cmd;
	unsigned short bit;
	unsigned short rpt;

	unsigned short decode_period;

	unsigned short rc5_data;
	unsigned short next_bit_is_zero;
	unsigned short pre_T;      /* Only RC5 use */
} rc_record_t;

typedef struct orionrc_ {
	spinlock_t lock;
	struct tasklet_struct tasklet;
	struct input_dev dev;

	unsigned short keycode[ORION_RC_KNUM];
	rc_record_t record;

	/* To be set outside, used as condition restriction */
	unsigned short addr;
	unsigned short max_rpt;
	unsigned short protocal;

#define FIFO_LEN 64
	unsigned short fifo1[FIFO_LEN];
	unsigned short fifo2[FIFO_LEN];
	short f1_idx;
	short f2_idx;

#ifdef CONFIG_RC_DEBUG
	unsigned short fc[32][2];	/* For debug only */
	short fc_idx;
#endif

	int bit_err;
} orionrc_t;

extern int orion_gpio_register_module_status(const char * module, unsigned int  orion_module_status); /* status: 0 disable, 1 enable */

#endif
