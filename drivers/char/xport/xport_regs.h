#ifndef __XPORT_REGS_H__
#define __XPORT_REGS_H__

#define XPORT_REG_ADDR_BASE         0x41400000

/******************HW Register define************************/
// address define
// xport_reg_pack block  addr: 0x00-0x1f
#define MAIL_BOX0_ADDR              (XPORT_REG_ADDR_BASE+(0x0000*4))
#define MAIL_BOX1_ADDR              (XPORT_REG_ADDR_BASE+(0x0001*4))
#define MAIL_BOX2_ADDR              (XPORT_REG_ADDR_BASE+(0x0002*4))
#define MAIL_BOX3_ADDR              (XPORT_REG_ADDR_BASE+(0x0003*4))
#define MAIL_BOX4_ADDR              (XPORT_REG_ADDR_BASE+(0x0004*4))
#define MAIL_BOX5_ADDR              (XPORT_REG_ADDR_BASE+(0x0005*4))
#define MAIL_BOX6_ADDR              (XPORT_REG_ADDR_BASE+(0x0006*4))
#define MAIL_BOX7_ADDR              (XPORT_REG_ADDR_BASE+(0x0007*4))

// bit0:   tuner0_mode,         0: Parallel mode    1: Serial
// bit1:   tuner1_mode,         0: Parallel mode    1: Serial
// bit2:   dma_in0_mode,        0: Normal DMA       1: DMA To Tuner
// bit4_3: dma_in0_speed_mode
// bit5:   dma_in1_mode,        0: Normal DMA       1: DMA To Tuner
// bit7_6: dma_in1_speed_mode
#define XPORT_CFG_ADDR0             (XPORT_REG_ADDR_BASE+(0x0008*4))

// bit3_0:     av_rp sync_mode,    0: host sync    1: line sync
#define XPORT_CFG_ADDR1             (XPORT_REG_ADDR_BASE+(0x0009*4))
#define XPORT_TUNER_STATES_ADDR     (XPORT_REG_ADDR_BASE+(0x000a*4))
#define XPORT_TUNER_STATES_CFG_ADDR (XPORT_REG_ADDR_BASE+(0x000b*4))
#define DMA_INPUT0_HEAD_ADDR        (XPORT_REG_ADDR_BASE+(0x000c*4))
#define DMA_INPUT1_HEAD_ADDR        (XPORT_REG_ADDR_BASE+(0x000d*4))

// bit0:  dma0 end interrupt
// bit1:  dma1 end interrupt                                                         
#define XPORT_INT_REG_ADDR0         (XPORT_REG_ADDR_BASE+(0x0010*4))
#define XPORT_INT_ENB_ADDR0         (XPORT_REG_ADDR_BASE+(0x0011*4))
#define XPORT_INT_SET_ADDR0         (XPORT_REG_ADDR_BASE+(0x0012*4))
#define XPORT_INT_CLS_ADDR0         (XPORT_REG_ADDR_BASE+(0x0013*4))

#define XPORT_INT_REG_ADDR1         (XPORT_REG_ADDR_BASE+(0x0014*4))
#define XPORT_INT_ENB_ADDR1         (XPORT_REG_ADDR_BASE+(0x0015*4))
#define XPORT_INT_SET_ADDR1         (XPORT_REG_ADDR_BASE+(0x0016*4))
#define XPORT_INT_CLS_ADDR1         (XPORT_REG_ADDR_BASE+(0x0017*4))

#define  XPORT_EXT_INT_ADDR   	    (MAIL_BOX5_ADDR)

// 0: tuner0 enable
// 1: tuner1 enable
#define  XPORT_TUNER_EN             (XPORT_REG_ADDR_BASE+(0x0018*4))

// xport_clock0 block     addr: 0x20-0x3fd
#define CLK0_BASE_ADDR               (XPORT_REG_ADDR_BASE+(0x0020*4))
#define CLK0_UP_ADDR                 (XPORT_REG_ADDR_BASE+(0x002a*4))
#define CLK0_STC_HIGH_ADDR           (XPORT_REG_ADDR_BASE+(0x0020*4))	// host:r
#define CLK0_STC_LOW_ADDR            (XPORT_REG_ADDR_BASE+(0x0021*4))	// host:r
#define CLK0_PCR_HIGH_ADDR           (XPORT_REG_ADDR_BASE+(0x0022*4))	// host:r   mips:w
#define CLK0_PCR_LOW_ADDR            (XPORT_REG_ADDR_BASE+(0x0023*4))	// host:r   mips:w
#define CLK0_PCR_CNT_ADDR            (XPORT_REG_ADDR_BASE+(0x0024*4))	// host:r
#define CLK0_PCR_INTERVAL_ADDR       (XPORT_REG_ADDR_BASE+(0x0025*4))	// host:r
#define CLK0_DELTA_CYCLES_ADDR       (XPORT_REG_ADDR_BASE+(0x0026*4))	// host:r
#define CLK0_DELTA_CYCLES_THRES_ADDR (XPORT_REG_ADDR_BASE+(0x0027*4))	// host:rw
#define CLK0_RESERVED                (XPORT_REG_ADDR_BASE+(0x0028*4))
#define CLK0_PWM_CTRL_ADDR           (XPORT_REG_ADDR_BASE+(0x0029*4))	// host:rw
#define CLK0_PWM_CWORD_ADDR          (XPORT_REG_ADDR_BASE+(0x002a*4))	// host:rw
#define CLK0_PCR_VALID_ADDR          (XPORT_REG_ADDR_BASE+(0x002b*4))	// mips:w
#define CLK0_REC_DELAY_TIME_ADDR     (XPORT_REG_ADDR_BASE+(0x002c*4))	// mips:rw
#define CLK0_CLK_COUTER_ADDR         (XPORT_REG_ADDR_BASE+(0x002d*4))	// mips:r

// xport_clock1 block     addr: 0x20-0x3fd
#define CLK1_BASE_ADDR               (XPORT_REG_ADDR_BASE+(0x0030*4))
#define CLK1_UP_ADDR                 (XPORT_REG_ADDR_BASE+(0x003a*4))
#define CLK1_STC_HIGH_ADDR           (XPORT_REG_ADDR_BASE+(0x0030*4))	// host:r
#define CLK1_STC_LOW_ADDR            (XPORT_REG_ADDR_BASE+(0x0031*4))	// host:r
#define CLK1_PCR_HIGH_ADDR           (XPORT_REG_ADDR_BASE+(0x0032*4))	// host:r   mips:w
#define CLK1_PCR_LOW_ADDR            (XPORT_REG_ADDR_BASE+(0x0033*4))	// host:r   mips:w
#define CLK1_PCR_CNT_ADDR            (XPORT_REG_ADDR_BASE+(0x0034*4))	// host:r
#define CLK1_PCR_INTERVAL_ADDR       (XPORT_REG_ADDR_BASE+(0x0035*4))	// host:r
#define CLK1_DELTA_CYCLES_ADDR       (XPORT_REG_ADDR_BASE+(0x0036*4))	// host:r
#define CLK1_DELTA_CYCLES_THRES_ADDR (XPORT_REG_ADDR_BASE+(0x0037*4))	// host:rw
#define CLK1_RESERVED                (XPORT_REG_ADDR_BASE+(0x0038*4))
#define CLK1_PWM_CTRL_ADDR           (XPORT_REG_ADDR_BASE+(0x0039*4))	// host:rw
#define CLK1_PWM_CWORD_ADDR          (XPORT_REG_ADDR_BASE+(0x003a*4))	// host:rw
#define CLK1_PCR_VALID_ADDR          (XPORT_REG_ADDR_BASE+(0x003b*4))	// mips:w
#define CLK1_REC_DELAY_TIME_ADDR     (XPORT_REG_ADDR_BASE+(0x003c*4))	// mips:rw
#define CLK1_CLK_COUTER_ADDR         (XPORT_REG_ADDR_BASE+(0x003d*4))	// mips:r

// xport_input_channel block addr:0x40-0x6f

// bit31: en,  bit30-29: type, bit28-27: sub type, bit26: chl_mode,  bit19-8: unit num, bit7-0: unit size,
// type: 0:tuner, 1:Eth, 2: DMA0, 3:DMA1   
// when type= tuner:
// sub type: 0: tuner 0,   1: tuner 1
// when type= Eth:
// sub type: 0:session0, 1:session1, 2:session2, 3:session3 
// when type= DMA0/1:
// sub type: data type

// #define __CHLx_CFG_ADDR__(i)		(XPORT_REG_ADDR_BASE+(0x0041 + (i*6))*4)

#define XPORT_CHL0_BASE_ADDR            (XPORT_REG_ADDR_BASE+(0x0040*4))
#define XPORT_CHL0_CFG_ADDR             (XPORT_REG_ADDR_BASE+(0x0041*4))
#define XPORT_CHL0_RP_ADDR              (XPORT_REG_ADDR_BASE+(0x0042*4))
#define XPORT_CHL0_WP_ADDR              (XPORT_REG_ADDR_BASE+(0x0043*4))
#define XPORT_CHL0_SEQ_ADDR             (XPORT_REG_ADDR_BASE+(0x0044*4))
#define XPORT_CHL0_STATUS_ADDR          (XPORT_REG_ADDR_BASE+(0x0045*4))

#define XPORT_CHL1_BASE_ADDR            (XPORT_REG_ADDR_BASE+(0x0046*4))
#define XPORT_CHL1_CFG_ADDR             (XPORT_REG_ADDR_BASE+(0x0047*4))
#define XPORT_CHL1_RP_ADDR              (XPORT_REG_ADDR_BASE+(0x0048*4))
#define XPORT_CHL1_WP_ADDR              (XPORT_REG_ADDR_BASE+(0x0049*4))
#define XPORT_CHL1_SEQ_ADDR             (XPORT_REG_ADDR_BASE+(0x004a*4))
#define XPORT_CHL1_STATUS_ADDR          (XPORT_REG_ADDR_BASE+(0x004b*4))

#define XPORT_CHL2_BASE_ADDR            (XPORT_REG_ADDR_BASE+(0x004c*4))
#define XPORT_CHL2_CFG_ADDR             (XPORT_REG_ADDR_BASE+(0x004d*4))
#define XPORT_CHL2_RP_ADDR              (XPORT_REG_ADDR_BASE+(0x004e*4))
#define XPORT_CHL2_WP_ADDR              (XPORT_REG_ADDR_BASE+(0x004f*4))
#define XPORT_CHL2_SEQ_ADDR             (XPORT_REG_ADDR_BASE+(0x0050*4))
#define XPORT_CHL2_STATUS_ADDR          (XPORT_REG_ADDR_BASE+(0x0051*4))

#define XPORT_CHL3_BASE_ADDR            (XPORT_REG_ADDR_BASE+(0x0052*4))
#define XPORT_CHL3_CFG_ADDR             (XPORT_REG_ADDR_BASE+(0x0053*4))
#define XPORT_CHL3_RP_ADDR              (XPORT_REG_ADDR_BASE+(0x0054*4))
#define XPORT_CHL3_WP_ADDR              (XPORT_REG_ADDR_BASE+(0x0055*4))
#define XPORT_CHL3_SEQ_ADDR             (XPORT_REG_ADDR_BASE+(0x0056*4))
#define XPORT_CHL3_STATUS_ADDR          (XPORT_REG_ADDR_BASE+(0x0057*4))

/*xunli: direct dma mode */
#define XPORT_CHL_DMA0_WP_ADDR          (MAIL_BOX2_ADDR)
#define XPORT_CHL_DMA1_WP_ADDR          (MAIL_BOX3_ADDR)

// PID filter reg  addr:  0x60-0x9f
#define __PID_FILTER__(i)             (XPORT_REG_ADDR_BASE+(0x0060 + i)*4)

#define PID_FILTER0                   (XPORT_REG_ADDR_BASE+(0x0060*4))
#define PID_FILTER1                   (XPORT_REG_ADDR_BASE+(0x0061*4))
#define PID_FILTER2                   (XPORT_REG_ADDR_BASE+(0x0062*4))
#define PID_FILTER3                   (XPORT_REG_ADDR_BASE+(0x0063*4))
#define PID_FILTER4                   (XPORT_REG_ADDR_BASE+(0x0064*4))
#define PID_FILTER5                   (XPORT_REG_ADDR_BASE+(0x0065*4))
#define PID_FILTER6                   (XPORT_REG_ADDR_BASE+(0x0066*4))
#define PID_FILTER7                   (XPORT_REG_ADDR_BASE+(0x0067*4))
#define PID_FILTER8                   (XPORT_REG_ADDR_BASE+(0x0068*4))
#define PID_FILTER9                   (XPORT_REG_ADDR_BASE+(0x0069*4))
#define PID_FILTER10                  (XPORT_REG_ADDR_BASE+(0x006a*4))
#define PID_FILTER11                  (XPORT_REG_ADDR_BASE+(0x006b*4))
#define PID_FILTER12                  (XPORT_REG_ADDR_BASE+(0x006c*4))
#define PID_FILTER13                  (XPORT_REG_ADDR_BASE+(0x006d*4))
#define PID_FILTER14                  (XPORT_REG_ADDR_BASE+(0x006e*4))
#define PID_FILTER15                  (XPORT_REG_ADDR_BASE+(0x006f*4))
#define PID_FILTER16                  (XPORT_REG_ADDR_BASE+(0x0070*4))
#define PID_FILTER17                  (XPORT_REG_ADDR_BASE+(0x0071*4))
#define PID_FILTER18                  (XPORT_REG_ADDR_BASE+(0x0072*4))
#define PID_FILTER19                  (XPORT_REG_ADDR_BASE+(0x0073*4))
#define PID_FILTER20                  (XPORT_REG_ADDR_BASE+(0x0074*4))
#define PID_FILTER21                  (XPORT_REG_ADDR_BASE+(0x0075*4))
#define PID_FILTER22                  (XPORT_REG_ADDR_BASE+(0x0076*4))
#define PID_FILTER23                  (XPORT_REG_ADDR_BASE+(0x0077*4))
#define PID_FILTER24                  (XPORT_REG_ADDR_BASE+(0x0078*4))
#define PID_FILTER25                  (XPORT_REG_ADDR_BASE+(0x0079*4))
#define PID_FILTER26                  (XPORT_REG_ADDR_BASE+(0x007a*4))
#define PID_FILTER27                  (XPORT_REG_ADDR_BASE+(0x007b*4))
#define PID_FILTER28                  (XPORT_REG_ADDR_BASE+(0x007c*4))
#define PID_FILTER29                  (XPORT_REG_ADDR_BASE+(0x007d*4))
#define PID_FILTER30                  (XPORT_REG_ADDR_BASE+(0x007e*4))
#define PID_FILTER31                  (XPORT_REG_ADDR_BASE+(0x007f*4))
#define PID_FILTER32                  (XPORT_REG_ADDR_BASE+(0x0080*4))
#define PID_FILTER33                  (XPORT_REG_ADDR_BASE+(0x0081*4))
#define PID_FILTER34                  (XPORT_REG_ADDR_BASE+(0x0082*4))
#define PID_FILTER35                  (XPORT_REG_ADDR_BASE+(0x0083*4))
#define PID_FILTER36                  (XPORT_REG_ADDR_BASE+(0x0084*4))
#define PID_FILTER37                  (XPORT_REG_ADDR_BASE+(0x0085*4))
#define PID_FILTER38                  (XPORT_REG_ADDR_BASE+(0x0086*4))
#define PID_FILTER39                  (XPORT_REG_ADDR_BASE+(0x0087*4))
#define PID_FILTER40                  (XPORT_REG_ADDR_BASE+(0x0088*4))
#define PID_FILTER41                  (XPORT_REG_ADDR_BASE+(0x0089*4))
#define PID_FILTER42                  (XPORT_REG_ADDR_BASE+(0x008a*4))
#define PID_FILTER43                  (XPORT_REG_ADDR_BASE+(0x008b*4))
#define PID_FILTER44                  (XPORT_REG_ADDR_BASE+(0x008c*4))
#define PID_FILTER45                  (XPORT_REG_ADDR_BASE+(0x008d*4))
#define PID_FILTER46                  (XPORT_REG_ADDR_BASE+(0x008e*4))
#define PID_FILTER47                  (XPORT_REG_ADDR_BASE+(0x008f*4))
#define PID_FILTER48                  (XPORT_REG_ADDR_BASE+(0x0090*4))
#define PID_FILTER49                  (XPORT_REG_ADDR_BASE+(0x0091*4))
#define PID_FILTER50                  (XPORT_REG_ADDR_BASE+(0x0092*4))
#define PID_FILTER51                  (XPORT_REG_ADDR_BASE+(0x0093*4))
#define PID_FILTER52                  (XPORT_REG_ADDR_BASE+(0x0094*4))
#define PID_FILTER53                  (XPORT_REG_ADDR_BASE+(0x0095*4))
#define PID_FILTER54                  (XPORT_REG_ADDR_BASE+(0x0096*4))
#define PID_FILTER55                  (XPORT_REG_ADDR_BASE+(0x0097*4))
#define PID_FILTER56                  (XPORT_REG_ADDR_BASE+(0x0098*4))
#define PID_FILTER57                  (XPORT_REG_ADDR_BASE+(0x0099*4))
#define PID_FILTER58                  (XPORT_REG_ADDR_BASE+(0x009a*4))
#define PID_FILTER59                  (XPORT_REG_ADDR_BASE+(0x009b*4))
#define PID_FILTER60                  (XPORT_REG_ADDR_BASE+(0x009c*4))
#define PID_FILTER61                  (XPORT_REG_ADDR_BASE+(0x009d*4))
#define PID_FILTER62                  (XPORT_REG_ADDR_BASE+(0x009e*4))
#define PID_FILTER63                  (XPORT_REG_ADDR_BASE+(0x009f*4))

// DESC KEY, addr: 0x180-0x1af 

#define __DESC_ODD_ADDR__(i, j)       (XPORT_REG_ADDR_BASE + 0x280 + 4 * 12 * i + 4 * j)
#define __DESC_EVEN_ADDR__(i, j)       (XPORT_REG_ADDR_BASE + 0x298 + 4 * 12 * i + 4 * j)

#define DESC0_ODD_KEY0                (XPORT_REG_ADDR_BASE+(0x00a0*4))
#define DESC0_ODD_KEY1                (XPORT_REG_ADDR_BASE+(0x00a1*4))
#define DESC0_ODD_KEY2                (XPORT_REG_ADDR_BASE+(0x00a2*4))
#define DESC0_ODD_KEY3                (XPORT_REG_ADDR_BASE+(0x00a3*4))
#define DESC0_ODD_KEY4                (XPORT_REG_ADDR_BASE+(0x00a4*4))
#define DESC0_ODD_KEY5                (XPORT_REG_ADDR_BASE+(0x00a5*4))
#define DESC0_EVEN_KEY0               (XPORT_REG_ADDR_BASE+(0x00a6*4))
#define DESC0_EVEN_KEY1               (XPORT_REG_ADDR_BASE+(0x00a7*4))
#define DESC0_EVEN_KEY2               (XPORT_REG_ADDR_BASE+(0x00a8*4))
#define DESC0_EVEN_KEY3               (XPORT_REG_ADDR_BASE+(0x00a9*4))
#define DESC0_EVEN_KEY4               (XPORT_REG_ADDR_BASE+(0x00aa*4))
#define DESC0_EVEN_KEY5               (XPORT_REG_ADDR_BASE+(0x00ab*4))
#define DESC1_ODD_KEY0                (XPORT_REG_ADDR_BASE+(0x00ac*4))
#define DESC1_ODD_KEY1                (XPORT_REG_ADDR_BASE+(0x00ad*4))
#define DESC1_ODD_KEY2                (XPORT_REG_ADDR_BASE+(0x00ae*4))
#define DESC1_ODD_KEY3                (XPORT_REG_ADDR_BASE+(0x00af*4))
#define DESC1_ODD_KEY4                (XPORT_REG_ADDR_BASE+(0x00b0*4))
#define DESC1_ODD_KEY5                (XPORT_REG_ADDR_BASE+(0x00b1*4))
#define DESC1_EVEN_KEY0               (XPORT_REG_ADDR_BASE+(0x00b2*4))
#define DESC1_EVEN_KEY1               (XPORT_REG_ADDR_BASE+(0x00b3*4))
#define DESC1_EVEN_KEY2               (XPORT_REG_ADDR_BASE+(0x00b4*4))
#define DESC1_EVEN_KEY3               (XPORT_REG_ADDR_BASE+(0x00b5*4))
#define DESC1_EVEN_KEY4               (XPORT_REG_ADDR_BASE+(0x00b6*4))
#define DESC1_EVEN_KEY5               (XPORT_REG_ADDR_BASE+(0x00b7*4))
#define DESC2_ODD_KEY0                (XPORT_REG_ADDR_BASE+(0x00b8*4))
#define DESC2_ODD_KEY1                (XPORT_REG_ADDR_BASE+(0x00b9*4))
#define DESC2_ODD_KEY2                (XPORT_REG_ADDR_BASE+(0x00ba*4))
#define DESC2_ODD_KEY3                (XPORT_REG_ADDR_BASE+(0x00bb*4))
#define DESC2_ODD_KEY4                (XPORT_REG_ADDR_BASE+(0x00bc*4))
#define DESC2_ODD_KEY5                (XPORT_REG_ADDR_BASE+(0x00bd*4))
#define DESC2_EVEN_KEY0               (XPORT_REG_ADDR_BASE+(0x00be*4))
#define DESC2_EVEN_KEY1               (XPORT_REG_ADDR_BASE+(0x00bf*4))
#define DESC2_EVEN_KEY2               (XPORT_REG_ADDR_BASE+(0x00c0*4))
#define DESC2_EVEN_KEY3               (XPORT_REG_ADDR_BASE+(0x00c1*4))
#define DESC2_EVEN_KEY4               (XPORT_REG_ADDR_BASE+(0x00c2*4))
#define DESC2_EVEN_KEY5               (XPORT_REG_ADDR_BASE+(0x00c3*4))
#define DESC3_ODD_KEY0                (XPORT_REG_ADDR_BASE+(0x00c4*4))
#define DESC3_ODD_KEY1                (XPORT_REG_ADDR_BASE+(0x00c5*4))
#define DESC3_ODD_KEY2                (XPORT_REG_ADDR_BASE+(0x00c6*4))
#define DESC3_ODD_KEY3                (XPORT_REG_ADDR_BASE+(0x00c7*4))
#define DESC3_ODD_KEY4                (XPORT_REG_ADDR_BASE+(0x00c8*4))
#define DESC3_ODD_KEY5                (XPORT_REG_ADDR_BASE+(0x00c9*4))
#define DESC3_EVEN_KEY0               (XPORT_REG_ADDR_BASE+(0x00ca*4))
#define DESC3_EVEN_KEY1               (XPORT_REG_ADDR_BASE+(0x00cb*4))
#define DESC3_EVEN_KEY2               (XPORT_REG_ADDR_BASE+(0x00cc*4))
#define DESC3_EVEN_KEY3               (XPORT_REG_ADDR_BASE+(0x00cd*4))
#define DESC3_EVEN_KEY4               (XPORT_REG_ADDR_BASE+(0x00ce*4))
#define DESC3_EVEN_KEY5               (XPORT_REG_ADDR_BASE+(0x00cf*4))
#define DESC4_ODD_KEY0                (XPORT_REG_ADDR_BASE+(0x00d0*4))
#define DESC4_ODD_KEY1                (XPORT_REG_ADDR_BASE+(0x00d1*4))
#define DESC4_ODD_KEY2                (XPORT_REG_ADDR_BASE+(0x00d2*4))
#define DESC4_ODD_KEY3                (XPORT_REG_ADDR_BASE+(0x00d3*4))
#define DESC4_ODD_KEY4                (XPORT_REG_ADDR_BASE+(0x00d4*4))
#define DESC4_ODD_KEY5                (XPORT_REG_ADDR_BASE+(0x00d5*4))
#define DESC4_EVEN_KEY0               (XPORT_REG_ADDR_BASE+(0x00d6*4))
#define DESC4_EVEN_KEY1               (XPORT_REG_ADDR_BASE+(0x00d7*4))
#define DESC4_EVEN_KEY2               (XPORT_REG_ADDR_BASE+(0x00d8*4))
#define DESC4_EVEN_KEY3               (XPORT_REG_ADDR_BASE+(0x00d9*4))
#define DESC4_EVEN_KEY4               (XPORT_REG_ADDR_BASE+(0x00da*4))
#define DESC4_EVEN_KEY5               (XPORT_REG_ADDR_BASE+(0x00db*4))
#define DESC5_ODD_KEY0                (XPORT_REG_ADDR_BASE+(0x00dc*4))
#define DESC5_ODD_KEY1                (XPORT_REG_ADDR_BASE+(0x00dd*4))
#define DESC5_ODD_KEY2                (XPORT_REG_ADDR_BASE+(0x00de*4))
#define DESC5_ODD_KEY3                (XPORT_REG_ADDR_BASE+(0x00df*4))
#define DESC5_ODD_KEY4                (XPORT_REG_ADDR_BASE+(0x00e0*4))
#define DESC5_ODD_KEY5                (XPORT_REG_ADDR_BASE+(0x00e1*4))
#define DESC5_EVEN_KEY0               (XPORT_REG_ADDR_BASE+(0x00e2*4))
#define DESC5_EVEN_KEY1               (XPORT_REG_ADDR_BASE+(0x00e3*4))
#define DESC5_EVEN_KEY2               (XPORT_REG_ADDR_BASE+(0x00e4*4))
#define DESC5_EVEN_KEY3               (XPORT_REG_ADDR_BASE+(0x00e5*4))
#define DESC5_EVEN_KEY4               (XPORT_REG_ADDR_BASE+(0x00e6*4))
#define DESC5_EVEN_KEY5               (XPORT_REG_ADDR_BASE+(0x00e7*4))
#define DESC6_ODD_KEY0                (XPORT_REG_ADDR_BASE+(0x00e8*4))
#define DESC6_ODD_KEY1                (XPORT_REG_ADDR_BASE+(0x00e9*4))
#define DESC6_ODD_KEY2                (XPORT_REG_ADDR_BASE+(0x00ea*4))
#define DESC6_ODD_KEY3                (XPORT_REG_ADDR_BASE+(0x00eb*4))
#define DESC6_ODD_KEY4                (XPORT_REG_ADDR_BASE+(0x00ec*4))
#define DESC6_ODD_KEY5                (XPORT_REG_ADDR_BASE+(0x00ed*4))
#define DESC6_EVEN_KEY0               (XPORT_REG_ADDR_BASE+(0x00ee*4))
#define DESC6_EVEN_KEY1               (XPORT_REG_ADDR_BASE+(0x00ef*4))
#define DESC6_EVEN_KEY2               (XPORT_REG_ADDR_BASE+(0x00f0*4))
#define DESC6_EVEN_KEY3               (XPORT_REG_ADDR_BASE+(0x00f1*4))
#define DESC6_EVEN_KEY4               (XPORT_REG_ADDR_BASE+(0x00f2*4))
#define DESC6_EVEN_KEY5               (XPORT_REG_ADDR_BASE+(0x00f3*4))
#define DESC7_ODD_KEY0                (XPORT_REG_ADDR_BASE+(0x00f4*4))
#define DESC7_ODD_KEY1                (XPORT_REG_ADDR_BASE+(0x00f5*4))
#define DESC7_ODD_KEY2                (XPORT_REG_ADDR_BASE+(0x00f6*4))
#define DESC7_ODD_KEY3                (XPORT_REG_ADDR_BASE+(0x00f7*4))
#define DESC7_ODD_KEY4                (XPORT_REG_ADDR_BASE+(0x00f8*4))
#define DESC7_ODD_KEY5                (XPORT_REG_ADDR_BASE+(0x00f9*4))
#define DESC7_EVEN_KEY0               (XPORT_REG_ADDR_BASE+(0x00fa*4))
#define DESC7_EVEN_KEY1               (XPORT_REG_ADDR_BASE+(0x00fb*4))
#define DESC7_EVEN_KEY2               (XPORT_REG_ADDR_BASE+(0x00fc*4))
#define DESC7_EVEN_KEY3               (XPORT_REG_ADDR_BASE+(0x00fd*4))
#define DESC7_EVEN_KEY4               (XPORT_REG_ADDR_BASE+(0x00fe*4))
#define DESC7_EVEN_KEY5               (XPORT_REG_ADDR_BASE+(0x00ff*4))

/******************MIPS software Register define************************/

#define MIPS_OUTCHL_EN                   0x0000
#define MIPS_OUTCHL_FILTER0              0x0001
#define MIPS_OUTCHL_FILTER1              0x0002
#define MIPS_OUTCHL_FILTER2              0x0003
#define MIPS_OUTCHL_MASK0                0x0004
#define MIPS_OUTCHL_MASK1                0x0005
#define MIPS_OUTCHL_MASK2                0x0006
#define MIPS_OUTCHL_DIR_LOW_ADRR         0x0007
#define MIPS_OUTCHL_DIR_UP_ADRR          0x0008
#define MIPS_OUTCHL_DIR_WP               0x0009
#define MIPS_OUTCHL_BUF_TC0_ERR_CNT      0x000a
#define MIPS_OUTCHL_BUF_TC1_ERR_CNT      0x000b
#define MIPS_OUTCHL_BUF_TC2_ERR_CNT      0x000c
#define MIPS_OUTCHL_BUF_TC3_ERR_CNT      0x000d
#define MIPS_OUTCHL_BUF_ERR_PACKET_CNT   0x000e
#define MIPS_OUTCHL_BUF_OUT_PACKET_CNT   0x000f
#define MIPS_OUTCHL_BUF_LOW_ADRR         0x0010
#define MIPS_OUTCHL_BUF_UP_ADRR          0x0011
#define MIPS_PCR_PID_ADDR                0x0012
#define MIPS_PCR_GET_ADDR                0x0013
#define MIPS_OUTCHL_CRC_EN               0x0014
#define MIPS_OUTCHL_CRC_NOTIFY_EN        0x0015

#define MIPS_OUTCHL_PID_BASE             0x0016

#define MIPS_OUTCHL_PID0                 (MIPS_OUTCHL_PID_BASE + 0x0)
#define MIPS_OUTCHL_PID1                 (MIPS_OUTCHL_PID_BASE + 0x1)
#define MIPS_OUTCHL_PID2                 (MIPS_OUTCHL_PID_BASE + 0x2)
#define MIPS_OUTCHL_PID3                 (MIPS_OUTCHL_PID_BASE + 0x3)
#define MIPS_OUTCHL_PID4                 (MIPS_OUTCHL_PID_BASE + 0x4)
#define MIPS_OUTCHL_PID5                 (MIPS_OUTCHL_PID_BASE + 0x5)
#define MIPS_OUTCHL_PID6                 (MIPS_OUTCHL_PID_BASE + 0x6)
#define MIPS_OUTCHL_PID7                 (MIPS_OUTCHL_PID_BASE + 0x7)
#define MIPS_OUTCHL_PID8                 (MIPS_OUTCHL_PID_BASE + 0x8)
#define MIPS_OUTCHL_PID9                 (MIPS_OUTCHL_PID_BASE + 0x9)
#define MIPS_OUTCHL_PID10                (MIPS_OUTCHL_PID_BASE + 0xa)
#define MIPS_OUTCHL_PID11                (MIPS_OUTCHL_PID_BASE + 0xb)

#define MIPS_OUTCHL_DISABLE		 (MIPS_OUTCHL_PID_BASE + 0x20)
#define MIPS_OUTCHL_SWITCH               (MIPS_OUTCHL_PID_BASE + 0x21)

#define MIPS_EXTERNAL_BASE               (MIPS_OUTCHL_PID_BASE + 0x22)

#define MIPS_OUTCHL_BITWISE0             (MIPS_OUTCHL_PID_BASE + 0x23)  
#define MIPS_OUTCHL_BITWISE1             (MIPS_OUTCHL_PID_BASE + 0x24)  
#define MIPS_OUTCHL_BITWISE2             (MIPS_OUTCHL_PID_BASE + 0x25)  

#endif

