#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

BOARD:=layerscape
BOARDNAME:=NXP Layerscape
DEVICE_TYPE:=developerboard
KERNEL_PATCHVER:=4.4
KERNELNAME:=Image dtbs
FEATURES:=squashfs nand usb pcie gpio
SUBTARGETS:=64b 32b
MAINTAINER:=Jiang Yutang <jiangyutang1978@gmail.com>

include $(INCLUDE_DIR)/target.mk

define Target/Description
	Build firmware images for $(BOARDNAME) SoC devices.
endef

DEFAULT_PACKAGES += kmod-usb3 kmod-usb-dwc3 kmod-usb-storage

$(eval $(call BuildTarget))
