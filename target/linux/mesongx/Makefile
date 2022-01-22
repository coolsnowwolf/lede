#
# Copyright (C) 2017-2019 Stijn Tintel <stijn@linux-ipv6.be>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

# ARCH:=aarch64
BOARD:=mesongx
BOARDNAME:=Amlogic
# CPU_TYPE:=cortex-a53
DEVICE_TYPE:=other
FEATURES:=audio boot-part ext4 fpu squashfs usbgadget
SUBTARGETS=m3 m6 m8 m8b m8m2 S905
MAINTAINER:=Stijn Tintel <stijn@linux-ipv6.be>

KERNEL_PATCHVER:=4.19

define Target/Description
	Build firmware image for Amlogic S9 SoC devices.
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
