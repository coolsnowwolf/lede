#
# Copyright (C) 2013-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2014.04
PKG_RELEASE:=1

PKG_HASH:=7b6444bd23eb61068c43bd1d44ec7e7bfdbce5cadeca20c833eee186b4d3fd31

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=imx6
  UBOOT_IMAGE:=u-boot.imx
endef

define U-Boot/mx6qsabresd
  NAME:=SABRE i.MX6Quad board
endef

define U-Boot/nitrogen6dl
  NAME:=Nitrogen6x i.MX6Dual-Lite 1GB board
endef

define U-Boot/nitrogen6dl2g
  NAME:=Nitrogen6x i.MX6Dual-Lite 2GB board
endef

define U-Boot/nitrogen6q
  NAME:=Nitrogen6x/SABRE Lite (MX6Q/1GB)
endef

define U-Boot/nitrogen6q2g
  NAME:=Nitrogen6x i.MX6Quad 2GB board
endef

define U-Boot/nitrogen6s
  NAME:=Nitrogen6x i.MX6Solo 512MB board
endef

define U-Boot/nitrogen6s1g
  NAME:=Nitrogen6x i.MX6Solo 1GB board
endef

define U-Boot/wandboard_dl
  NAME:=Wandboard Dual Lite
  BUILD_DEVICES:=wandboard
endef

define U-Boot/wandboard_quad
  NAME:=Wandboard Quad
  BUILD_DEVICES:=wandboard
endef

define U-Boot/wandboard_solo
  NAME:=Wandboard Solo
  BUILD_DEVICES:=wandboard
endef

UBOOT_TARGETS := \
	mx6qsabresd \
	nitrogen6dl \
	nitrogen6dl2g \
	nitrogen6q \
	nitrogen6q2g \
	nitrogen6s \
	nitrogen6s1g \
	wandboard_dl \
	wandboard_quad \
	wandboard_solo

$(eval $(call BuildPackage/U-Boot))
