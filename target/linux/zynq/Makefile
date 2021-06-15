# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2015 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=zynq
BOARDNAME:=Xilinx Zynq 7000 SoCs
FEATURES:=fpu gpio rtc usb usbgadget boot-part rootfs-part squashfs
CPU_TYPE:=cortex-a9
CPU_SUBTYPE:=neon
MAINTAINER:=Jason Wu <jason.wu.misc@gmail.com>

# future support SUBTARGETS: for both zynq and zynqmp

define Target/Description
	Build firmware image for Zynq 7000 SoC devices.
endef

KERNEL_PATCHVER:=5.4

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=Image dtbs

DEFAULT_PACKAGES += uboot-envtools mkf2fs e2fsprogs \
	kmod-usb-storage kmod-fs-msdos

$(eval $(call BuildTarget))
