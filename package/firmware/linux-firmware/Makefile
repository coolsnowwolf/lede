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
PKG_SOURCE_DATE:=2018-08-03
PKG_SOURCE_VERSION:=7b5835fd37630d18ac0c755329172f6a17c1af29
PKG_MIRROR_HASH:=fd631e7d024b0da78ad930a64503f816be19f49b15bbd454e83d41fd8dfde793
PKG_SOURCE_URL:=git://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git

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
