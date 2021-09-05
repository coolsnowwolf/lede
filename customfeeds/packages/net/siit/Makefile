#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=siit
PKG_VERSION:=1.2
PKG_RELEASE:=3
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define KernelPackage/siit
  SUBMENU:=Network Devices
  TITLE:=Stateless IP ICMP Translation Algorithm
  DEPENDS:= @(!(TARGET_ps3||TARGET_pxcab))
  FILES:=$(PKG_BUILD_DIR)/siit.ko
  AUTOLOAD:=$(call AutoLoad,50,siit)
  MAINTAINER:=Vladimir Ulrich <admin@evl.su>
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define KernelPackage/siit/description
 Stateless IP ICMP Translation Algorithm
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp src/Makefile src/siit.h src/siit.c $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(KERNEL_MAKEOPTS) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,siit))
