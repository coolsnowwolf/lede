# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2008 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=bcm47xx
BOARDNAME:=Broadcom BCM47xx/53xx (MIPS)
FEATURES:=squashfs usb
SUBTARGETS:=generic mips74k legacy
MAINTAINER:=Hauke Mehrtens <hauke@hauke-m.de>

KERNEL_PATCHVER:=5.4

define Target/Description
	Build firmware images for Broadcom based BCM47xx/53xx routers with MIPS CPU, *not* ARM.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += swconfig nvram otrx \
	kmod-leds-gpio kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
