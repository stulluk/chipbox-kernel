# CHIPBOX Linux Kernel

This is the linux kernel for Chipbox satellite receiver.

[What is "Chipbox"](https://gitlab.com/chipbox/about-chipbox)

## Prequisities & Building


- You need a toolchain. Chipbox has a very ancient toolchain here:
https://gitlab.com/stulluk/chipbox-pars/-/tree/main/crosstool/gcc-3.4.6-glibc-2.3.6/arm-linux
- Clone that repository and add this toolchain to your $PATH ( this will give you arm-linux-gcc )
- As of writing this README, I tried almost all toolchains out there, none of them can 
compile a simple printf("hello,world\n") statically, allways return segmentation fault. I don't know why.
- There are two convenience shell scripts in this dir: ```ba``` (to build) and ```ca``` (to clean)
- .gitignore is up to date
- .config is included and works on the real hardware
- When build completes, it creates a file under arch/arm/boot/uImage as follows:
```
$  file arch/arm/boot/uImage
arch/arm/boot/uImage: u-boot legacy uImage, Linux-2.6.12.5, Linux/ARM, OS Kernel Image (Not compressed), 1581924 bytes, Sun Dec 26 20:57:19 2021, Load Address: 0x00008000, Entry Point: 0x00008000, Header CRC: 0x43826AFE, Data CRC: 0xF2960D8D
stulluk /tmp/chipbox-kernel (master) $
```

## Running

- Setup a TFTPD server on your ubuntu host
- On Chipbox, enter u-boot shell by pressing enter just after you power on
- Then execute followings:
```
========    CHIPBOX Bootloader Started    ========
 0
CHIPBOX > 
CHIPBOX > 
CHIPBOX > printenv
bootdelay=0
baudrate=115200
bootcmd=bootm 0x380A0000
filesize=4729c
fileaddr=5000000
logo=0x38080000 435 65 130 250
bootargs=root=/dev/mtdblock/2 rootfstype=jffs2 mem=146M console=ttyS0,115200 mtdparts=phys_mapped_flash:640k(u-boot0),1792k(kernel),5760k(fs),384k(u-boot1),5120k(plugin),2688k(appl),384k(uboot2),7808k(w)
ethaddr=02:03:04:61:72:A8
serverip=192.168.1.212
ipaddr=192.168.1.232
stdin=serial
stdout=serial
stderr=serial
verify=n
CHIPBOX > tftpboot 0x0000000 uImage
TFTP from server 192.168.1.212; our IP address is 192.168.1.232

Loading: #################################################################
         #################################################################
         #################################################################
         #################################################################
         #################################################
done
CHIPBOX > bootm
   Image Type:   OK
Uncompressing Linux.................................................................................................... done, booting the kernel.
Linux version 2.6.12.5 (stulluk@amdpc) (gcc version 3.4.6) #76 Sun Dec 26 23:21:01 +03 2021
CPU: ARM926EJ-Sid(wb) [41069265] revision 5 (ARMv5TEJ)
CPU0: D VIVT write-back cache
CPU0: I cache: 16384 bytes, associativity 4, 32 byte lines, 128 sets
CPU0: D cache: 16384 bytes, associativity 4, 32 byte lines, 128 sets
Machine: ORION
Memory policy: ECC disabled, Data cache writeback
Built 1 zonelists
Kernel command line: root=/dev/mtdblock/2 rootfstype=jffs2 mem=146M console=ttyS0,115200 mtdparts=phys_mapped_flash:640k(u-boot0),1792k(kernel),5760k(fs),384k(u-boot1),5120k(plugin),2688k(appl),384k(ubo8
ORION Readed MAC:02:03:04:61:72:a8
PID hash table entries: 1024 (order: 10, 16384 bytes)
Console: colour dummy device 80x30
Dentry cache hash table entries: 32768 (order: 5, 131072 bytes)
Inode-cache hash table entries: 16384 (order: 4, 65536 bytes)
Memory: 146MB = 146MB total
Memory: 144768KB available (2462K code, 642K data, 96K init)
Mount-cache hash table entries: 512
CPU: Testing write buffer coherency: ok
NET: Registered protocol family 16
SCSI subsystem initialized
Linux Kernel Card Services
  options:  none
usbcore: registered new driver usbfs
usbcore: registered new driver hub
NetWinder Floating Point Emulator V0.97 (extended precision)
devfs: 2004-01-31 Richard Gooch (rgooch@atnf.csiro.au)
devfs: boot_options: 0x0
NTFS driver 2.1.22 [Flags: R/O].
JFFS2 version 2.2. (NAND) (C) 2001-2003 Red Hat, Inc.
fb0: ORION frame buffer @[0xf400000, 0xc9880000] size 0xc00000
fb1: ORION frame buffer @[0xe800000, 0xca500000] size 0xc00000
ORION GPIO at 0x101e4000, 16 lines
ORION GPIO2 at 0x10260000, 55 lines
 Smart card: base address 101f0000 map to c9864000
Orion Watchdog Timer: timer margin 40 sec
Xport: Init OK [0x0e100000]. 
orion_video: Orion Video driver was initialized, at address@[phyical addr = 41600000, size = 100000] 
csdrv audio init ok ...
Default ORION I2C at 0x10170000, 100KHZ
orion15_df: Orion Display feeder driver was initialized, at address@[phyical addr = 41800000, size = 1000] 
orion15_df: Orion TVE0 driver was initialized, at address@[phyical addr = 10168000, size = 1000] 
orion15_df: Orion TVE1 driver was initialized, at address@[phyical addr = 10160000, size = 1000] 
pinmux0 : 0x800, pinmux1: 0x100d
dma_pool_alloc: dma_phy_addr: 917e000, ttx_buf: ffc00000
Serial: 8250/16550 driver $Revision: 1.90 $ 2 ports, IRQ sharing disabled
ttyS0 at MMIO 0x101f1000 (irq = 12) is a 16550A
ttyS1 at MMIO 0x101f2000 (irq = 13) is a 16550A
io scheduler noop registered
io scheduler anticipatory registered
RAMDISK driver initialized: 1 RAM disks of 16384K size 1024 blocksize
loop: loaded (max 8 devices)
ORION eth0: 0x41400000 IRQ 24 MAC:02:03:04:61:72:a8
Not SST flash.<5>physmap flash device: 2000000 at 38000000
phys_mapped_flash: Found 1 x16 devices at 0x0 in 16-bit bank
 Amd/Fujitsu Extended Query Table at 0x0040
phys_mapped_flash: CFI does not contain boot bank location. Assuming top.
number of CFI chips: 1
cfi_cmdset_0002: Disabling erase-suspend-program due to code brokenness.
10 cmdlinepart partitions found on MTD device phys_mapped_flash
Creating 10 MTD partitions on "phys_mapped_flash":
0x00000000-0x000a0000 : "u-boot0"
0x000a0000-0x00260000 : "kernel"
0x00260000-0x00800000 : "fs"
0x00800000-0x00860000 : "u-boot1"
0x00860000-0x00d60000 : "plugin"
0x00d60000-0x01000000 : "appl"
0x01000000-0x01060000 : "uboot2"
0x01060000-0x01800000 : "work0"
0x01800000-0x01860000 : "uboot3"
0x01860000-0x02000000 : "work1"
No NAND device found!!!
No NAND device - returning -ENXIO
orion socket ci module initializing!!!
Initializing USB Mass Storage driver...
usbcore: registered new driver usb-storage
USB Mass Storage support registered.
usbcore: registered new driver usbhid
drivers/usb/input/hid-core.c: v2.01:USB HID core driver
drivers/usb/musb/musb_driver.c: Initializing MUSB Driver [npci=1][gadget=no][otg=no]
MUSB Drive version 1.1.0.1
mgc_controller_init: Driver instance data at 0xc91a5e00
mgc_hdrc_init: MHDRC version 1.500  info: UTMI-8, dyn FIFOs, SoftConn
mgc_controller_init: End 00: FIFO TxSize=0040/RxSize=0040
mgc_controller_init: End 01: Shared FIFO TxSize=0200/RxSize=0200
mgc_controller_init: End 02: FIFO TxSize=0000/RxSize=0040
mgc_controller_init: End 03: Shared FIFO TxSize=0400/RxSize=0400
mgc_controller_init: New bus @0xc916c820
musb-hcd usb0: new USB bus registered, assigned bus number 1
hub 1-0:1.0: USB hub found
hub 1-0:1.0: 1 port detected
KB hub_activate
NET: Registered protocol family 2
IP: routing cache hash table of 1024 buckets, 8Kbytes
TCP established hash table entries: 8192 (order: 4, 65536 bytes)
TCP bind hash table entries: 8192 (order: 3, 32768 bytes)
TCP: Hash tables configured (established 8192 bind 8192)
NET: Registered protocol family 1
NET: Registered protocol family 17
NET: Registered protocol family 15
VFS: Mounted root (jffs2 filesystem).
Freeing init memory: 96K
Warning: unable to open an initial console.
ORION eth0: MII transceiver 1 status 0x786d advertising 01e1.
eth0: link up, 100Mbps, full-duplex, lpa 0xC5E1
# uname -a
Linux Chipbox 2.6.12.5 #76 Sun Dec 26 23:21:01 +03 2021 armv5tejl GNU/Linux
# loading xport firmware
 readed xport fw data section size =236
readed xport fw text section size =11004
loded xport firmware

# 
```