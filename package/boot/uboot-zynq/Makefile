#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2016.03

PKG_HASH:=e49337262ecac44dbdeac140f2c6ebd1eba345e0162b0464172e7f05583ed7bb

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define U-Boot/Default
  BUILD_TARGET:=zynq
  UBOOT_IMAGE:=u-boot u-boot-dtb.bin u-boot.dtb u-boot-dtb.img spl/boot.bin spl/u-boot-spl spl/u-boot-spl-dtb.bin
  UBOOT_CONFIG:=zynq_$(1)
endef

define U-Boot/zc702
  NAME:=Xilinx ZC702 Dev Board
endef

define U-Boot/zc706
  NAME:=Xilinx ZC706 Dev Board
endef

define U-Boot/zed
  NAME:=Avnet Digilent ZedBoard Dev Board
endef

define U-Boot/zybo
  NAME:=Digilent Zybo Dev Board
endef

UBOOT_TARGETS := \
	zc702 \
	zed \
	zybo

Build/Exports:=$(Host/Exports)

$(eval $(call BuildPackage/U-Boot))
