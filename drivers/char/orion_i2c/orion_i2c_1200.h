#ifndef __ORION_I2C_H__
#define __ORION_I2C_H__

/* Register Map */
#define     IC_CON          	0x00
#define     IC_TAR          	0x04
#define     IC_SAR          	0x08
#define     IC_DATA_CMD     	0x10
#define     IC_SS_SCL_HCNT  	0x14
#define     IC_SS_SCL_LCNT  	0x18
#define     IC_FS_SCL_HCNT  	0x1C
#define     IC_FS_SCL_LCNT  	0x20
#define     IC_RAW_INTR_STAT    0x34
#define	    IC_CLR_TX_ABRT	0x54
#define	    IC_CLR_STOP_DET	0x60
#define     IC_ENABLE       	0x6C
#define     IC_STATUS       	0x70
#define     IC_COMP_TYPE    	0xfc
#define     IC_TX_ABRT_SOURCE   0x80

#endif
