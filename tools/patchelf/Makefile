# 
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=patchelf
PKG_VERSION:=0.9

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://nixos.org/releases/patchelf/patchelf-$(PKG_VERSION)
PKG_HASH:=a0f65c1ba148890e9f2f7823f4bedf7ecad5417772f64f994004f59a39014f83

HOST_BUILD_PARALLEL:=1
HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/src/patchelf $(STAGING_DIR_HOST)/bin/patchelf
endef

$(eval $(call HostBuild))
