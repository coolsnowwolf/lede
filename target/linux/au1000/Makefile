#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=au1000
BOARDNAME:=RMI/AMD AU1x00
FEATURES:=squashfs usb pci small_flash source-only
SUBTARGETS:=au1500 au1550
MAINTAINER:=Florian Fainelli <florian@openwrt.org>

KERNEL_PATCHVER:=3.18

define Target/Description
	Build firmware for RMI/AMD Alchemy 1500,1550 boards
	(e.g. 4G-Systems Mesh/Access Cube, DBAu1550 ...)
endef

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += wpad-mini yamonenv

define Kernel/BuildImage
	$(call Kernel/BuildImage/Default)
	$(CP) $(LINUX_DIR)/arch/mips/boot/compressed/images/zImage.flash.srec $(LINUX_DIR)/zImage.flash.srec
	$(CP) $(LINUX_DIR)/arch/mips/boot/compressed/images/zImage.srec $(LINUX_DIR)/zImage.srec
endef

$(eval $(call BuildTarget))
