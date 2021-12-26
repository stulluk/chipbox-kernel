====================
Building the Driver:
====================

1. Go to your kernel source tree, subdirectory drivers/usb:

# cd <your-kernel-source-tree>/drivers/usb

2. Make a link to the musb driver, called "musb":

# ln -s <musb-driver-location> musb

3. Add this line before the "endmenu" at the end of Kconfig:

source "drivers/usb/musb/Kconfig"

4. Add this line at the end of Makefile:

obj-$(CONFIG_USB_INVENTRA_HCD)  += musb/

5. Configure your kernel (e.g. make menuconfig),
   enabling the MUSB driver under drivers -> USB Support,
   and then proceed as normal (see your kernel's README)
