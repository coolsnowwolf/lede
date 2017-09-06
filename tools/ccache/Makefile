# 
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=ccache
PKG_VERSION:=3.3.4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://download.samba.org/pub/ccache/ \
		https://samba.org/ftp/ccache/
PKG_HASH:=24f15bf389e38c41548c9c259532187774ec0cb9686c3497bbb75504c8dc404f

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += CC="$(HOSTCC_NOCACHE)"

define Host/Install/ccache
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin/
	$(CP) ./files/* $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	-$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
endef
define Host/Install
	$(call Host/Install/Default)
	$(call Host/Install/ccache)
endef

$(eval $(call HostBuild))
