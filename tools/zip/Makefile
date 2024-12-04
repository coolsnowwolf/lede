#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zip
PKG_REV:=30
PKG_VERSION:=3.0

PKG_SOURCE:=$(PKG_NAME)$(PKG_REV).tar.gz
PKG_SOURCE_URL:=@SF/infozip
PKG_HASH:=f0e8bb1f9b7eb0b01285495a2699df3a4b766784c1765a8f1aeedf63c0806369

PKG_LICENSE:=BSD-4-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:zip_project:zip

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)/zip$(PKG_REV)
HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	+$(HOST_MAKE_VARS) $(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) -I. -f unix/Makefile zip
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/zip $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -rf $(STAGING_DIR_HOST)/bin/zip
endef

$(eval $(call HostBuild))
