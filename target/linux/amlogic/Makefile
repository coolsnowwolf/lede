# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2013 OpenWrt.org

include $(TOPDIR)/rules.mk

BOARD:=amlogic
BOARDNAME:=Amlogic
BOARDNAME:=Amlogic Meson family
FEATURES:=boot-part ext4 audio usb fpu legacy-sdcard squashfs usbgadget display gpio fpu pci pcie
SUBTARGETS:=meson8b mesongx

KERNEL_PATCHVER:=6.1

define Target/Description
	Build firmware image for Amlogic Meson SoC devices.
	Currently produces SD Card/USB disk image.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	e2fsprogs \
	mkf2fs \
	automount \
	resize2fs \
	partx-utils \
	losetup htop autocore-arm

KERNELNAME:=Image dtbs

$(eval $(call BuildTarget))
