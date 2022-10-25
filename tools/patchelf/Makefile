# 
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=patchelf

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NixOS/patchelf.git
PKG_SOURCE_VERSION:=f34751b88bd07d7f44f5cd3200fb4122bf916c7e
PKG_SOURCE_DATE:=2020-12-07
PKG_MIRROR_HASH:=ac746930b919b97da40f259cfc9ab7bbd48a0c9cbf2eebd8cee5ae19a94356fd

HOST_BUILD_PARALLEL:=1
HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/src/patchelf $(STAGING_DIR_HOST)/bin/patchelf
endef

define Host/Clean
	rm -rf $(STAGING_DIR_HOST)/bin/patchelf
endef

$(eval $(call HostBuild))
