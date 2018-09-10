#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

BOARD:=layerscape
BOARDNAME:=NXP Layerscape
KERNEL_PATCHVER:=4.9
FEATURES:=squashfs nand usb pcie gpio fpu ubifs
SUBTARGETS:=armv8_64b armv8_32b
MAINTAINER:=Yangbo Lu <yangbo.lu@nxp.com>

define Target/Description
	Build firmware images for NXP Layerscape based boards.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += kmod-usb3 kmod-usb-dwc3 kmod-usb-storage

$(eval $(call BuildTarget))
