# 
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=pkg-config
PKG_VERSION:=0.29.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pkgconfig.freedesktop.org/releases/
PKG_HASH:=6fc69c01688c9458a57eb9a1664c9aba372ccda420a02bf4429fe610e7e7d591

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

unexport PKG_CONFIG

HOSTCC := $(HOSTCC_NOCACHE)

HOST_CONFIGURE_ARGS += --with-internal-glib

ifeq ($(HOST_OS),Darwin)
HOST_LDFLAGS += -framework CoreFoundation -framework Carbon
endif

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
	mv $(STAGING_DIR_HOST)/bin/pkg-config $(STAGING_DIR_HOST)/bin/pkg-config.real
	$(INSTALL_BIN) ./files/pkg-config $(STAGING_DIR_HOST)/bin/pkg-config
endef

define Host/Clean
	-$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
endef

$(eval $(call HostBuild))
