#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=fortify-headers
PKG_VERSION:=0.8
PKG_RELEASE=1

PKG_SOURCE_URL:=http://dl.2f30.org/releases
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=afcd073d6d8d175eede2a28a7dd71b7150cb03290f99102261892c793f6b4cb1

include $(INCLUDE_DIR)/toolchain-build.mk

define Host/Compile
	true
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) PREFIX="" DESTDIR="$(TOOLCHAIN_DIR)" install
endef

$(eval $(call HostBuild))
