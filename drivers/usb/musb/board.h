/******************************************************************
 * Copyright 2006 Mentor Graphics Corporation
 *
 * This file is part of the Inventra Controller Driver for Linux.
 *
 * The Inventra Controller Driver for Linux is free software; you
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License version 2 as published by the Free Software
 * Foundation.
 *
 * The Inventra Controller Driver for Linux is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with The Inventra Controller Driver for Linux ; if not,
 * write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA
 *
 * ANY DOWNLOAD, USE, REPRODUCTION, MODIFICATION OR DISTRIBUTION
 * OF THIS DRIVER INDICATES YOUR COMPLETE AND UNCONDITIONAL ACCEPTANCE
 * OF THOSE TERMS.THIS DRIVER IS PROVIDED "AS IS" AND MENTOR GRAPHICS
 * MAKES NO WARRANTIES, EXPRESS OR IMPLIED, RELATED TO THIS DRIVER.
 * MENTOR GRAPHICS SPECIFICALLY DISCLAIMS ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY; FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  MENTOR GRAPHICS DOES NOT PROVIDE SUPPORT
 * SERVICES OR UPDATES FOR THIS DRIVER, EVEN IF YOU ARE A MENTOR
 * GRAPHICS SUPPORT CUSTOMER.
 ******************************************************************/

/*
 * Example board-specific definitions.
 * $Revision: 1.1.1.1 $
 */

#ifndef __MGC_BOARD_H__
#define __MGC_BOARD_H__

/*
 * It is suggested to:
 * 1. Copy this file to one named after your target:
 *    cp board.h board-mytarget.h
 * 2. Save this file for future reference:
 *    mv board.h board-example.h
 * 3. Link board.h to yours:
 *    ln -s board-mytarget.h board.h
 * 4. Edit yours, providing, for each controller:
 *    - controller type (MGC_CONTROLLER_HDRC or MGC_CONTROLLER_MHDRC)
 *    - physical base address in kernel space
 *    - interrupt number (interpretation is platform-specific)
 */

/*
 * The defines below allow adding platform-specific behavior
 * to key areas of the driver without needing to modify the
 * shipped driver.
 */

/** called before all controller initializations */
#if defined (CONFIG_ARCH_ORION_CSM1100) /* for orion_1.3 */

#define MGC_EXTRA_INIT()  do {                                                  \
        volatile unsigned short *gpio_base;                                     \
        volatile unsigned short *idscs_base;                                    \
                                                                                \
        gpio_base = (volatile unsigned short *)ioremap(0x101E4000, 0x100);      \
        gpio_base[0x008 >> 1] = gpio_base[0x008 >> 1] & 0xfff7;                 \
        gpio_base[0x004 >> 1] = gpio_base[0x004 >> 1] | 0x0008;                 \
        gpio_base[0x000 >> 1] = gpio_base[0x000 >> 1] & 0xfff7;                 \
        iounmap((void *)gpio_base);                                             \
                                                                                \
        idscs_base = (volatile unsigned short *)ioremap(0x10171000, 0x1000);    \
        idscs_base[0x400 >> 1] = idscs_base[0x400 >> 1] | 0x0002;               \
        idscs_base[0x200 >> 1] = idscs_base[0x200 >> 1] & 0xfdff;               \
        udelay(20);                                                             \
        idscs_base[0x200 >> 1] = idscs_base[0x200 >> 1] | 0x0200;               \
        iounmap((void *)idscs_base);                                            \
}while(0)

#elif defined (CONFIG_ARCH_ORION_CSM1200) /* for orion_1.4 */

#define MGC_EXTRA_INIT()  do {                                                  \
        volatile unsigned short *idscs_base;                                    \
        idscs_base = (volatile unsigned short *)ioremap(0x10171000, 0x1000);    \
        idscs_base[0x200 >> 1] = idscs_base[0x200 >> 1] & 0xfdff;               \
        udelay(20);                                                             \
        idscs_base[0x200 >> 1] = idscs_base[0x200 >> 1] | 0x0200;               \
        iounmap((void *)idscs_base);                                            \
        udelay(100);                                                            \
}while(0)

#else /* for csm1201 */

#define MGC_EXTRA_INIT() do {} while(0)

#endif

/** called before core initialization */
#define MGC_EXTRA_PRE_INIT(_nIrq, _pRegs)

/** called after core initialization */
#define MGC_EXTRA_POST_INIT(_pThis) do { mgc_setting_ulpi(_pThis); }while(0)

/** called just before starting core */
#define MGC_EXTRA_START(_pThis)

/** called at the start of ISR */
#define MGC_EXTRA_PRE_ISR(_pThis)

/** called at the end of ISR */
#define MGC_EXTRA_POST_ISR(_pThis)

/** Array of information about hard-wired controllers */
MGC_LinuxController MGC_aLinuxController[] = {
        { MGC_CONTROLLER_MHDRC, (void*)0x10200000, 22, 0x10000 }
};

#endif	/* __MGC_BOARD_H__ */

