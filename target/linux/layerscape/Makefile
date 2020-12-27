#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

BOARD:=layerscape
BOARDNAME:=NXP Layerscape
KERNEL_PATCHVER:=5.4
FEATURES:=squashfs nand usb pcie gpio fpu ubifs ext4 rootfs-part boot-part
SUBTARGETS:=armv8_64b armv7

define Target/Description
	Build firmware images for NXP Layerscape based boards.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += kmod-usb3 kmod-usb-dwc3 kmod-usb-storage \
  mkf2fs e2fsprogs partx-utils

$(eval $(call BuildTarget))
