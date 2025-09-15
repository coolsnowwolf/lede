#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=pkgconf
PKG_VERSION:=2.2.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://distfiles.dereferenced.org/pkgconf
PKG_HASH:=28f8dfc279a10ef66148befa3f6eb266e5f3570316600208ed50e9781c7269d8

PKG_CPE_ID:=cpe:/a:pkgconf:pkgconf

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/meson.mk

unexport PKG_CONFIG

HOSTCC := $(HOSTCC_NOCACHE)

MESON_HOST_ARGS += \
	-Ddefault_library=static \
	-Dtests=disabled

define Host/Install
	$(call Host/Install/Meson)
	mv $(STAGING_DIR_HOST)/bin/pkgconf $(STAGING_DIR_HOST)/bin/pkg-config.real
	$(INSTALL_BIN) ./files/pkg-config $(STAGING_DIR_HOST)/bin/pkg-config
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/pkg-config.real $(STAGING_DIR_HOST)/bin/pkg-config
	$(call Host/Clean/Meson)
endef

$(eval $(call HostBuild))
