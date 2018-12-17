#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=linux-firmware
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2018-12-16
PKG_SOURCE_VERSION:=211de1679a68b8ab0f841a8058df35e13e3963f0
PKG_MIRROR_HASH:=894a13a44c9de58528488682feb3ade582725cd766a16ccaca07089628229d90
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

SCAN_DEPS = *.mk

include $(INCLUDE_DIR)/package.mk

define Package/firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=http://git.kernel.org/cgit/linux/kernel/git/firmware/linux-firmware.git
  TITLE:=$(1)
endef

define Build/Compile

endef

include $(wildcard ./*.mk)
#$(eval $(call BuildPackage,linux-firmware))
