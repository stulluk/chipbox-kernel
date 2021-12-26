
#ifndef __XPORT_MIPS_H__
#define __XPORT_MIPS_H__

#include "xport_regs.h"

#define MIPS_CHL_EN(x)                  (MIPS_OUTCHL_EN+((x)<<8))
#define MIPS_CHL_PID0(x)                (MIPS_OUTCHL_PID0+((x)<<8))
#define MIPS_CHL_PID1(x)                (MIPS_OUTCHL_PID1+((x)<<8))
#define MIPS_CHL_PID2(x)                (MIPS_OUTCHL_PID2+((x)<<8))
#define MIPS_CHL_PID3(x)                (MIPS_OUTCHL_PID3+((x)<<8))
#define MIPS_CHL_PID4(x)                (MIPS_OUTCHL_PID4+((x)<<8))
#define MIPS_CHL_PID5(x)                (MIPS_OUTCHL_PID5+((x)<<8))
#define MIPS_CHL_PID6(x)                (MIPS_OUTCHL_PID6+((x)<<8))
#define MIPS_CHL_PID7(x)                (MIPS_OUTCHL_PID7+((x)<<8))
#define MIPS_CHL_PID8(x)                (MIPS_OUTCHL_PID8+((x)<<8))
#define MIPS_CHL_PID9(x)                (MIPS_OUTCHL_PID9+((x)<<8))
#define MIPS_CHL_PID10(x)               (MIPS_OUTCHL_PID10+((x)<<8))
#define MIPS_CHL_PID11(x)               (MIPS_OUTCHL_PID11+((x)<<8))
#define MIPS_CHL_FILTER0(x)             (MIPS_OUTCHL_FILTER0+((x)<<8))
#define MIPS_CHL_FILTER1(x)             (MIPS_OUTCHL_FILTER1+((x)<<8))
#define MIPS_CHL_FILTER2(x)             (MIPS_OUTCHL_FILTER2+((x)<<8))
#define MIPS_CHL_MASK0(x)               (MIPS_OUTCHL_MASK0+((x)<<8))
#define MIPS_CHL_MASK1(x)               (MIPS_OUTCHL_MASK1+((x)<<8))
#define MIPS_CHL_MASK2(x)               (MIPS_OUTCHL_MASK2+((x)<<8))
#define MIPS_CHL_DIR_LOW_ADDR(x)        (MIPS_OUTCHL_DIR_LOW_ADRR+((x)<<8))
#define MIPS_CHL_DIR_UP_ADDR(x)         (MIPS_OUTCHL_DIR_UP_ADRR+((x)<<8))
#define MIPS_CHL_WP(x)                  (MIPS_OUTCHL_DIR_WP+((x)<<8))
#define MIPS_CHL_TC0_ERR_CNT(x)         (MIPS_OUTCHL_BUF_TC0_ERR_CNT+((x)<<8))
#define MIPS_CHL_TC1_ERR_CNT(x)         (MIPS_OUTCHL_BUF_TC1_ERR_CNT+((x)<<8))
#define MIPS_CHL_TC2_ERR_CNT(x)         (MIPS_OUTCHL_BUF_TC2_ERR_CNT+((x)<<8))
#define MIPS_CHL_TC3_ERR_CNT(x)         (MIPS_OUTCHL_BUF_TC3_ERR_CNT+((x)<<8))
#define MIPS_CHL_ERR_PKT_CNT(x)         (MIPS_OUTCHL_BUF_ERR_PACKET_CNT+((x)<<8))
#define MIPS_CHL_OUT_PKT_CNT(x)         (MIPS_OUTCHL_BUF_OUT_PACKET_CNT+((x)<<8))
#define MIPS_CHL_BUF_LOW_ADDR(x)        (MIPS_OUTCHL_BUF_LOW_ADRR+((x)<<8))
#define MIPS_CHL_BUF_UP_ADDR(x)         (MIPS_OUTCHL_BUF_UP_ADRR+((x)<<8))
#define MIPS_PCR_PID(x)                 (MIPS_PCR_PID_ADDR+((x)<<8))
#define MIPS_PCR_GET(x)                 (MIPS_PCR_GET_ADDR+((x)<<8))
#define MIPS_CHL_CRC_EN(x)              (MIPS_OUTCHL_CRC_EN+((x)<<8))
#define MIPS_CHL_CRC_NOTIFY_EN(x)       (MIPS_OUTCHL_CRC_NOTIFY_EN+((x)<<8))
#define MIPS_CHL_DISABLE(x)		(MIPS_OUTCHL_DISABLE + ((x)<<8))
#define MIPS_CHL_SWITCH(x)		(MIPS_OUTCHL_SWITCH + ((x)<<8))

#define  MIPS_FW_WRITE_DATA  		(0xc1400000+(0x4000<<2))
#define  MIPS_FW_WRITE_INST  		(0xc1400000+(0x2000<<2))
#define  MIPS_FW_EN   			(0xc1400000+(0x6000<<2))

#define MIPS_CMD_REQ 	MAIL_BOX0_ADDR
#define MIPS_CMD_DATA	MAIL_BOX1_ADDR
#define MIPS_CMD_DATA2	MAIL_BOX4_ADDR

union firmware_req {

	struct {
		unsigned int req_type:8;
		unsigned int output_idx:8;
		unsigned int reserved:14;
		unsigned int rw:1;
		unsigned int enable:1;
	} bits;

	unsigned int val;
};

int xport_mips_write(unsigned int cmd, unsigned int req_dat);
int xport_mips_read(unsigned int cmd, unsigned int *req_dat);
int xport_mips_read_ex(unsigned int cmd, unsigned int *req_dat, unsigned int *req_dat2);

#endif
