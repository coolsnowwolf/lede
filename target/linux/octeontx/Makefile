#
# Copyright (C) 2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=octeontx
BOARDNAME:=Octeon-TX
FEATURES:=targz pcie gpio rtc usb fpu

MAINTAINER:=Tim Harvey <tharvey@gateworks.com>

KERNEL_PATCHVER:=4.14

define Target/Description
	Build images for Octeon-TX CN80XX/CN81XX based boards
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=Image

$(eval $(call BuildTarget))
