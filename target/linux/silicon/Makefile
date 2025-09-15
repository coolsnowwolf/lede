# SPDX-License-Identifier: GPL-2.0-or-later
#

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=silicon
BOARDNAME:=Apple Silicon family
FEATURES:=audio boot-part ext4 fpu squashfs usbgadget
SUBTARGETS:=armv8

KERNEL_PATCHVER:=6.1

define Target/Description
	Build firmware image for Apple Silicon family
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