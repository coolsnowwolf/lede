# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2017-2021 Stijn Tintel <stijn@linux-ipv6.be>

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=meson
BOARDNAME:=Amlogic Meson family
FEATURES:=audio boot-part ext4 fpu legacy-sdcard squashfs usbgadget
MAINTAINER:=Stijn Tintel <stijn@linux-ipv6.be>
SUBTARGETS:=meson8b mesongx

KERNEL_PATCHVER:=5.10

define Target/Description
	Build firmware image for Amlogic Meson SoC devices.
	Currently produces SD Card/USB disk image.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	e2fsprogs \
	kmod-sound-core \
	kmod-usb-hid \
	mkf2fs \
	partx-utils

KERNELNAME:=Image dtbs

$(eval $(call BuildTarget))
