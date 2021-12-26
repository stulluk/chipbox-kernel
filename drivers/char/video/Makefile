#
# Makefile for the kernel orion1.4 device drivers.
#

ifdef CONFIG_ARCH_ORION_CSM1201
obj-$(CONFIG_VIDEO_SUPPORT) += orion_vid.o
endif
ifdef CONFIG_ARCH_ORION_CSM1200
obj-$(CONFIG_VIDEO_SUPPORT) += orion_vid.o orion_df.o
obj-$(CONFIG_TVE_SUPPORT) += orion_tve.o
endif

