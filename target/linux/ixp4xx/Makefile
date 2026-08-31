# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2023 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=armeb
BOARD:=ixp4xx
BOARDNAME:=Intel XScale IXP4xx
FEATURES:=dt squashfs gpio ext4 rootfs-part
CPU_TYPE:=xscale
SUBTARGETS:=generic

KERNEL_PATCHVER:=6.18

define Target/Description
	Build firmware images for the IXP4xx XScale CPU
endef

KERNELNAME:=zImage dtbs

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += fconfig \
	kmod-crypto-hw-ixp4xx \
	kmod-usb-ledtrig-usbport \
	kmod-leds-gpio \
	kmod-input-gpio-keys

$(eval $(call BuildTarget))
