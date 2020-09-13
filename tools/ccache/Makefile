# 
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=ccache
PKG_VERSION:=3.7.9

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/ccache/ccache/releases/download/v$(PKG_VERSION)
PKG_HASH:=f893da7543bfb9172bd55e603fcbdfcd83e6def176a28689c13235695b4cf44b

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
