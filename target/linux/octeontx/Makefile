# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2018 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=octeontx
BOARDNAME:=Octeon-TX
FEATURES:=targz pcie gpio rtc usb fpu

MAINTAINER:=Tim Harvey <tharvey@gateworks.com>

KERNEL_PATCHVER:=5.4

define Target/Description
	Build images for Octeon-TX CN80XX/CN81XX based boards
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=Image

$(eval $(call BuildTarget))
