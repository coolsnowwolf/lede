# 
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=quilt
PKG_VERSION:=0.65

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SAVANNAH/quilt
PKG_HASH:=f6cbc788e5cbbb381a3c6eab5b9efce67c776a8662a7795c7432fd27aa096819

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
	cd $(HOST_BUILD_DIR) && autoconf
	$(call Host/Configure/Default)
	[ -f $(HOST_BUILD_DIR)/Makefile ]
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) SHELL="$(BASH)" all
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) SHELL="$(BASH)" install
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/quilt
endef

$(eval $(call HostBuild))
