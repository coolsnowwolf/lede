# Copyright (c) 2015 OpenWrt.org
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=mediatek
BOARDNAME:=MediaTek Ralink ARM
SUBTARGETS:=mt7622 mt7623 mt7629
FEATURES:=squashfs nand ramdisk fpu
MAINTAINER:=John Crispin <john@phrozen.org>

KERNEL_PATCHVER:=4.19

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-mt76 kmod-leds-gpio kmod-gpio-button-hotplug \
	wpad-basic

$(eval $(call BuildTarget))
