#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2018.07

PKG_HASH:=9f10df88bc91b35642e461217f73256bbaeeca9ae2db8db56197ba5e89e1f6d4

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define U-Boot/Default
  BUILD_TARGET:=zynq
  UBOOT_IMAGE:=spl/boot.bin u-boot.img
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

define U-Boot/zybo_z7
  NAME:=Digilent Zybo Z7 board
endef

UBOOT_TARGETS := \
	zc702 \
	zed \
	zybo \
	zybo_z7

Build/Exports:=$(Host/Exports)

$(eval $(call BuildPackage/U-Boot))
