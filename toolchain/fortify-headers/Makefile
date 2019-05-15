#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=fortify-headers
PKG_VERSION:=1.0
PKG_RELEASE=1

PKG_SOURCE_URL:=http://dl.2f30.org/releases
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=026e4cc0f2aa69b7922b1bc8838491048b497faf1c8b20450741114f9ea3775b

include $(INCLUDE_DIR)/toolchain-build.mk

define Host/Compile
	true
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) PREFIX="" DESTDIR="$(TOOLCHAIN_DIR)" install
endef

$(eval $(call HostBuild))
